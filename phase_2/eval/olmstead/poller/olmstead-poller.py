#!/usr/bin/env python3

import sys
import socket
import select
import os
import random
import threading
import subprocess
import string
import base64

from pwn import *

person_attributes = [ b'telephoneNumber', b'description']
organizationalPerson_attribs = [b'title', b'registeredAddress', b'destinationIndicator', b'telephoneNumber', b'facsimileTelephoneNumber', b'street', b'postOfficeBox', b'postalCode', b'postalAddress', b'physicalDeliveryOfficeName', b'st', b'l']
inetOrgPerson_attribs = [b'audio', b'businessCategory', b'carLicense', b'departmentNumber', b'displayName', b'employeeNumber', b'employeeType', b'givenName', b'homePhone', b'homePostalAddress', b'initials', b'jpegPhoto', b'labeledURI', b'mail', b'mobile', b'o', b'pager', b'roomNumber' ]

def create_listener( port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.bind(('', port))
    except socket.error as msg:
        print('[ERROR] Failed to bind on port {0}: {1} - {2}'.format(port, msg[0], msg[1]))
        sys.exit(1)

    s.listen(5)

    return s

def do_connection( host, port):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect((host, port))
    except socket.error as msg:
        print('[ERROR] Failed to connect to {0}:{1} - {2} - {3}'.format(host, port, msg[0], msg[1]))
        sys.exit(1)

    return s

def getlink_by_fd( links, s):
    for x in links:
        if s in x:
            return x

    return None

## Generates a random string of length l
def gen_random_string( l ):
    z = ''

    while len(z) < l:
        z += random.choice( string.ascii_lowercase + string.ascii_uppercase + string.digits)

    return bytes(z, 'UTF-8')

## Writes an ldif file to temp
def write_ldif( data ):

    try:
        f = open('/tmp/nl.ldif', 'wb')
    except:
        print('[ERROR] Failed to open /tmp/nl.ldif for writing')
        sys.exit(1)

    f.write(data)

    f.close()

def parse_ldif( location ):
    try:
        f = open(location, 'rb')
    except:
        print('[ERROR] Failed to open %s' %(location))
        sys.exit()

    data = f.read()
    f.close()

    data = data.replace(b'::', b':')

    lines = data.split(b'\n')

    attribs = {}

    for l in lines:
        if l.startswith(b'#') == True:
            continue

        if l == b'':
            continue;

        at = l.split(b':')

        if len(at) != 2:
            print('[ERROR] Invalid data in ldif file: %s' %(l))
            sys.exit(0)

        attribs[at[0]] = at[1].lstrip(b' ')

    return attribs

## generates the ldif to add an attribute
def genadd( dn, attrib, value ):
    ldif = b''

    ldif += b'dn: ' + dn + b'\n'

    ldif += b'changetype: modify\n'
    ldif += b'add: ' + attrib + b'\n'
    ldif += attrib + b': ' + value + b'\n'

    return ldif

def gendel( dn, attrib ):
    ldif = b''

    ldif += b'dn: ' + dn + b'\n'

    ldif += b'changetype: modify\n'
    ldif += b'delete: ' + attrib + b'\n'

    return ldif

def genrep( dn, attrib, value ):
    ldif = b''

    ldif += b'dn: ' + dn + b'\n'

    ldif += b'changetype: modify\n'
    ldif += b'replace: ' + attrib + b'\n'
    ldif += attrib + b': ' + value + b'\n'

    return ldif

def run_proxy( listen_port, host, port):
    ## Initialize the listening server
    server = create_listener( listen_port )
    server.setblocking(0)

    inputs = [ server ]
    outputs = [ ]

    links = []

    while inputs:
        readable, writable, exceptional = select.select( inputs, outputs, inputs )

        for s in readable:
            ## Accept the connection
            if s is server:
                cl, cla = s.accept()

                print("[INFO] Accepted client: {0}".format(cla) )

                cl.setblocking(0)

                inputs.append(cl)

                ## connect to the slapd server
                se = do_connection( host, port )

                print("[INFO] Connected client to server")

                inputs.append(se)
                links.append([ cl, se])

            else:
                l = getlink_by_fd( links, s)

                data = s.recv(1024)

                print('[INFO] Received data')

                if data:
                    if ( l[0] == s ):
                        l[1].sendall(data)
                    else:
                        l[0].sendall(data)
                else:
                    l[0].close()
                    l[1].close()
                    inputs.remove(l[0])
                    inputs.remove(l[1])

                    links.remove(l)

        for s in exceptional:
            l = getlink_by_fd( links, s)

            l[0].close()
            l[1].close()

            inputs.remove(l[0])
            inputs.remove(l[1])

            links.remove(l)

def ls_oc( host, port, attribs):
    print('[TEST] ldapsearch(by objectClass)')
    # ./ldapsearch -x -D "cn=admin,dc=chess,dc=com" -w secret -b "dc=chess,dc=com" objectClass=?

    args = ['ldapsearch', '-h', host, '-p', port, '-x', '-D', 'uid=chess,ou=people,dc=chess,dc=com', '-w', 'chess', '-b', 'dc=chess,dc=com']

    cl = random.choice(['top', 'person', 'organizationalPerson', 'inetOrgPerson', '*'])

    args.append('objectClass=' + cl)

    print('\t[INFO] objectClass=' + cl)

    try:
        stdout = subprocess.check_output(args, stderr=subprocess.STDOUT).rstrip(b'\n')
    except subprocess.CalledProcessError as e:
        stdout = e.output.rstrip(b'\n')

    if stdout.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    return 0

def ls_cf( host, port, attribs):
    print('[TEST] ldapsearch(limit fields)')
    # ./ldapsearch -x -D "cn=admin,dc=chess,dc=com" -w secret -b "dc=chess,dc=com" objectClass=?

    args = ['ldapsearch', '-h', host, '-p', port, '-x', '-D', 'uid=chess,ou=people,dc=chess,dc=com', '-w', 'chess', '-b', 'dc=chess,dc=com']

    all_attribs = list(attribs.keys())

    allowed_attribs = list(set(all_attribs).intersection( set(person_attributes + organizationalPerson_attribs + inetOrgPerson_attribs)))

    for _ in range(random.randint(1, len(allowed_attribs))):
        at = random.choice(allowed_attribs)

        print('\t[INFO] Adding attribute: %s' %(at))

        args.append(at)
        allowed_attribs.remove(at)
    
    p = process(args)

    res = p.recv()

    p.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    print(res)

    return 0

def ls_and( host, port, attribs):
    print('[TEST] ldapsearch(and test)')
    # ./ldapsearch -x -D "cn=admin,dc=chess,dc=com" -w secret -b "dc=chess,dc=com" objectClass=?

    args = ['ldapsearch', '-h', host, '-p', port, '-x', '-D', 'uid=chess,ou=people,dc=chess,dc=com', '-w', 'chess', '-b', 'dc=chess,dc=com']

    all_attribs = list(attribs.keys())
    all_attribs.sort()

    allowed_attribs = list(set(all_attribs).intersection( set(person_attributes + organizationalPerson_attribs + inetOrgPerson_attribs)))
    allowed_attribs.sort()

    oc = random.choice([b'top', b'person', b'organizationalPerson', b'inetOrgPerson', b'*'])

    conditions = '(&(objectClass=' + oc.decode('UTF-8') + ')'

    for _ in range(random.randint(1, len(allowed_attribs))):
        at = random.choice(allowed_attribs)

        print('\t[INFO] Adding attribute: %s' %(at))

        val = attribs[at].decode('UTF-8').lstrip()

        if val.find(' ') != -1:
            val = val[: val.find(' ')] + '*'

        val = val.replace('(', '\\(')
        val = val.replace(')', '\\)')

        conditions += '(' + at.decode('UTF-8') + '=' + val + ')'
        allowed_attribs.remove(at)
    
    conditions += ')'

    args.append(conditions)

    print(args)

    p = process(args)

    res = p.recv()

    p.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    print(res)

    return 0

def ls_or( host, port, attribs):
    print('[TEST] ldapsearch(or test)')
    # ./ldapsearch -x -D "cn=admin,dc=chess,dc=com" -w secret -b "dc=chess,dc=com" objectClass=?

    args = ['ldapsearch', '-h', host, '-p', port, '-x', '-D', 'uid=chess,ou=people,dc=chess,dc=com', '-w', 'chess', '-b', 'dc=chess,dc=com']

    all_attribs = list(attribs.keys())
    all_attribs.sort()

    allowed_attribs = list(set(all_attribs).intersection( set(person_attributes + organizationalPerson_attribs + inetOrgPerson_attribs)))
    allowed_attribs.sort()

    oc = random.choice([b'top', b'person', b'organizationalPerson', b'inetOrgPerson', b'*'])

    conditions = '"(|(objectClass=' + oc.decode('UTF-8') + ')'

    for _ in range(random.randint(1, len(allowed_attribs))):
        at = random.choice(allowed_attribs)

        print('\t[INFO] Adding attribute: %s' %(at))

        val = attribs[at].decode('UTF-8').lstrip()

        if val.find(' ') != -1:
            val = val[: val.find(' ')] + '*'

        val = val.replace('(', '\\(')
        val = val.replace(')', '\\)')

        conditions += '(' + at.decode('UTF-8') + '=' + val + ')'
        allowed_attribs.remove(at)
    
    conditions += ')"'

    args.append(conditions)

    p = process(args)

    res = p.recv()

    p.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    return 0

def ls_ast( attribs):
    pass

def ls_base( host, port, attribs):
    print('[TEST] ldapsearch(base case)')
    args = ['ldapsearch', '-h', host, '-p', port, '-x', '-D', 'uid=chess,ou=people,dc=chess,dc=com', '-w', 'chess', '-b', 'dc=chess,dc=com']

    p = process(args)

    res = p.recv()

    p.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    print(res)

    return 0

def ldapsearch( host, port, alice_attribs, bob_attribs ):

    # ./ldapsearch -x -D "uid=chess,ou=people,dc=chess,dc=com" -w chess -b "dc=chess,dc=com"

    ## limit by objectClass
    ## just display certain fields
    ## & operator
    ## | operator
    ## ! operator
    ## asterix

    sel = random.choice( [ alice_attribs, bob_attribs] )

    ### modify existing
    ### add / delete entry
    ### add / increment / delete

    f = random.choice([ ls_or, ls_and, ls_cf, ls_oc, ls_base])

    return f( host, port, sel )


def lm_mod( host, port, attribs ):
    print('[TEST] ldapmodify (replace)')
    # select an attribute to modify
    a = ''

    ## get a list of the attributes
    all_attribs = list(attribs.keys())
    all_attribs.sort()

    ## I only want the set of those attributes that are already associatied with the entry along with those I am willing to modify
    ### as specified in the global attribute lists
    allowed_attribs = set(all_attribs).intersection( set(person_attributes + organizationalPerson_attribs + inetOrgPerson_attribs))
    allowed_attribs = list(allowed_attribs)
    allowed_attribs.sort()

    ## Randomly select one of the valid attributes
    selected_attrib = random.choice( allowed_attribs )

    ## Save the orginal value
    original_value = attribs[selected_attrib]

    new_value = gen_random_string( random.randint( 5, 15) )

    print('\t[INFO] Changing %s from %s to %s' %(selected_attrib, original_value, new_value))

    ## Generate the new ldif data
    new_ldif = genrep( attribs[b'dn'], selected_attrib, new_value )

    ## Write the new ldif data to a file. I hard coded /tmp/nl.dif
    write_ldif( new_ldif )

    args = ['ldapmodify', '-h', host, '-p', port, '-x', '-D', 'uid=chess,ou=people,dc=chess,dc=com', '-w', 'chess', '-f', '/tmp/nl.ldif'  ]

    print(args)

    try:
        stdout = subprocess.check_output(args, stderr=subprocess.STDOUT).rstrip(b'\n')
    except subprocess.CalledProcessError as e:
        stdout = e.output.rstrip(b'\n')

    print(stdout)

    if stdout.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    result = b'modifying entry "' + attribs[b'dn'] + b'"'

    if result != stdout:
        print(f'[FAIL] ldapmodify (replace) failed expected: {result} received: {stdout}')
        return 1

    print('\t[INFO] Returning %s from %s to %s' %(selected_attrib, new_value, original_value))
    ## Now we need to return the entry to its original value to maintain testing continuity
    orig_ldif = genrep( attribs[b'dn'], selected_attrib, original_value )    

    print(orig_ldif)

    write_ldif( orig_ldif )

    try:
        stdout = subprocess.check_output(args, stderr=subprocess.STDOUT).rstrip(b'\n')
    except subprocess.CalledProcessError as e:
        stdout = e.output.rstrip(b'\n')

    if stdout.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    if result != stdout:
        print(f'[FAIL] ldapmodify (replace) failed expected: {result} received: {stdout}')
        return 1
    else:
        print(f'[SUCCESS] ldapmodify (replace) succeeded')
        return 0

def lm_add( host, port, attribs ):
    print('[TEST] ldapmodify (add)')
    # select an attribute to modify
    a = ''

    ## get a list of the attributes
    all_attribs = list(attribs.keys())
    all_attribs.sort()

    ## I want the set of attributes not already assigned to the person
    allowed_attribs = set( set(person_attributes + organizationalPerson_attribs + inetOrgPerson_attribs) - set(all_attribs))
    allowed_attribs = list(allowed_attribs)
    allowed_attribs.sort()

    ## Randomly select one of the valid attributes
    selected_attrib = random.choice( allowed_attribs )

    new_value = gen_random_string( random.randint( 5, 15) )

    print('\t[INFO] Adding attribute %s: %s' %(selected_attrib, new_value))

    ## Generate the new ldif data
    new_ldif = genadd( attribs[b'dn'], selected_attrib, new_value )

    print(new_ldif)

    ## Write the new ldif data to a file. I hard coded /tmp/nl.dif
    write_ldif( new_ldif )

    args = ['ldapmodify', '-h', host, '-p', port, '-x', '-D', 'uid=chess,ou=people,dc=chess,dc=com', '-w', 'chess', '-f', '/tmp/nl.ldif', '-a'  ]

    print(args)

    try:
        stdout = subprocess.check_output(args, stderr=subprocess.STDOUT).rstrip(b'\n')
    except subprocess.CalledProcessError as e:
        stdout = e.output.rstrip(b'\n')

    if stdout.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    result = b'modifying entry "' + attribs[b'dn'].lstrip(b' ') + b'"'

    if result != stdout:
        print(f'[FAIL] ldapmodify (add) failed expected: {result} received: {stdout}')
        return 1

    print('\t[INFO] Removing attribute: %s' %(selected_attrib))
    ## Now we need to return the entry to its original value to maintain testing continuity
    orig_ldif = gendel( attribs[b'dn'], selected_attrib )    

    print(orig_ldif)
    write_ldif( orig_ldif )
    
    print(args)

    try:
        stdout = subprocess.check_output(args, stderr=subprocess.STDOUT).rstrip(b'\n')
    except subprocess.CalledProcessError as e:
        stdout = e.output.rstrip(b'\n')

    if stdout.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    if result != stdout:
        print(f'[FAIL] ldapmodify (add) failed expected: {result} received: {stdout}')
        return 1
    else:
        print(f'[SUCCESS] ldapmodify (add) succeeded')
        return 0

def lm_inc( host, port, attribs ):
    pass

## select either yolo or hj and modify their attributes
def ldapmodify( host, port, alice_attribs, bob_attribs ):
    sel = random.choice( [ alice_attribs, bob_attribs] )

    ### modify existing
    ### add / delete entry
    ### add / increment / delete

    f = random.choice([ lm_add, lm_mod])

    return f( host, port, sel )

def ldapwhoami( host, port, alice_attribs, bob_attribs):
    sel = random.choice( [ alice_attribs, bob_attribs] )

    args = ['ldapwhoami', '-h', host, '-p', port, '-x', '-D', sel[b'dn'], '-w', base64.b64decode(sel[b'userPassword']) ]

    p = process(args)

    res = p.recv().rstrip(b'\n')

    p.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    result = b'dn:' + sel[b'dn'].lstrip(b' ') + b''

    if res == result:
        print('[SUCCESS] ldapwhoami succeeded')
        return 0
    else:
        print(f'[FAIL] ldapwhoami failed expected: {result} received: {res}')
        return 1

def format_attrib( attrib, value, r=False):

    a = b''

    if attrib == b'telephoneNumber':
        if r:
            a += b'%03d-%03d-%04d' %(random.randint(0,999), random.randint(0,999), random.randint(0,9999))
            return a
        else:
            return value
    elif attrib == b'l':
        if r:
            a += b'%s' %(gen_random_string(6))
            return a
        else:
            a = b'%s' %(value)
            return a
    else:
        if r:
            return gen_random_string(6)
        else:
            return value

def ldapcompare( host, port, alice_attribs, bob_attribs):
    print('[TEST] ldapcompare')

    sel = random.choice( [ alice_attribs, bob_attribs] )

    args = ['ldapcompare', '-h', host, '-p', port, '-x', '-D', 'uid=chess,ou=people,dc=chess,dc=com', '-w', 'chess', sel[b'dn'] ]

    ## get a list of the attributes
    all_attribs = list(sel.keys())
    all_attribs.sort()

    ## I only want the set of those attributes that are already associatied with the entry along with those I am willing to modify
    ### as specified in the global attribute lists
    allowed_attribs = set(all_attribs).intersection( set(person_attributes + organizationalPerson_attribs + inetOrgPerson_attribs))
    allowed_attribs = list(allowed_attribs)
    allowed_attribs.sort()

    ## Randomly select one of the valid attributes
    selected_attrib = random.choice( allowed_attribs )

    result = ''

    if random.randint( 0, 100) > 50:
        args.append( selected_attrib + b':' + format_attrib(selected_attrib, sel[selected_attrib]) ) 
        result = b'TRUE'
    else:
        args.append( selected_attrib + b':' + format_attrib(selected_attrib, '', True) )
        result = b'FALSE'

    print(args)

    try:
        stdout = subprocess.check_output(args, stderr=subprocess.STDOUT).rstrip(b'\n')
    except subprocess.CalledProcessError as e:
        stdout = e.output.rstrip(b'\n')

    if stdout.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    if stdout == result:
        print('[SUCCESS] compare succeeded')
        return 0
    else:
        print(f'[FAIL] compare failed: expected: {result} received: {stdout}')
        return 1

def run_test( host, port ):
    alice_attribs = parse_ldif( './alicea.ldif' )

    bob_attribs = parse_ldif( './bob.ldif' )

    f = random.choice( [ldapcompare, ldapwhoami, ldapmodify, ldapsearch])
    return f( host, port, alice_attribs, bob_attribs)

if __name__ == '__main__':
    if len(sys.argv) == 4:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
        TOOLS = sys.argv[3]
    else:
        if 'HOST' in os.environ and 'PORT':
            HOST = os.environ['HOST']
            PORT = os.environ['PORT']
        else:
            print('[USAGE] %s <host> <port>' %(sys.argv[0]))
            print('[INFO] This poller acts as a launcher for the openldap client tools against the olmstead server.')
            print('       The location of the client tools must be')
            print('       specified via the path argument. You can also use the environment')
            print('       variables HOST, and PORT in lieu of command line args.')
            sys.exit(1)

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('[INFO] Poller for the Olmstead Challenge')
    print('[INFO] SEED: {0}'.format(SEED))

    random.seed( SEED )

    for x in range(30):
        result = run_test(HOST, PORT)

        if result:
            print('[FAIL] Poller failed')
            exit(1)

    print('[SUCCESS] Poller was successful')
    exit(0)
