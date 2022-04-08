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
organizationalPerson_attribs = [b'title', b'registeredAddress', b'destinationIndicator', b'telexNumber', b'telephoneNumber', b'facsimileTelephoneNumber', b'street', b'postOfficeBox', b'postalCode', b'postalAddress', b'physicalDeliveryOfficeName', b'st', b'l']
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

        attribs[at[0]] = at[1]

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

def ls_oc( host, port, base_dir, attribs):
    print('[TEST] ldapsearch(by objectClass)')
    # ./ldapsearch -x -D "cn=admin,dc=chess,dc=com" -w secret -b "dc=chess,dc=com" objectClass=?

    args = [base_dir + 'ldapsearch', '-h', host, '-p', port, '-x', '-D', 'cn=admin,dc=chess,dc=com', '-w', 'secret', '-b', 'dc=chess,dc=com']

    cl = random.choice(['top', 'person', 'organizationalPerson', 'inetOrgPerson', '*'])

    args.append('objectClass=' + cl)

    print('\t[INFO] objectClass=' + cl)

    p = ssh(host='challenge_container', user='chess', password='chess')

    sh = p.process(args)

    res = sh.recv()

    sh.close()
    p.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    print(res)

    return 0

def ls_cf( host, port, base_dir, attribs):
    print('[TEST] ldapsearch(limit fields)')
    # ./ldapsearch -x -D "cn=admin,dc=chess,dc=com" -w secret -b "dc=chess,dc=com" objectClass=?

    args = [base_dir + 'ldapsearch', '-h', host, '-p', port, '-x', '-D', 'cn=admin,dc=chess,dc=com', '-w', 'secret', '-b', 'dc=chess,dc=com']

    all_attribs = list(attribs.keys())

    allowed_attribs = list(set(all_attribs).intersection( set(person_attributes + organizationalPerson_attribs + inetOrgPerson_attribs)))

    for _ in range(random.randint(1, len(allowed_attribs))):
        at = random.choice(allowed_attribs)

        print('\t[INFO] Adding attribute: %s' %(at))

        args.append(at)
        allowed_attribs.remove(at)
    
    p = ssh(host='challenge_container', user='chess', password='chess')

    sh = p.process(args)

    res = sh.recv()

    sh.close()
    p.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    print(res)

    return 0

def ls_and( host, port, base_dir, attribs):
    print('[TEST] ldapsearch(and test)')
    # ./ldapsearch -x -D "cn=admin,dc=chess,dc=com" -w secret -b "dc=chess,dc=com" objectClass=?

    args = [base_dir + 'ldapsearch', '-h', host, '-p', port, '-x', '-D', 'cn=admin,dc=chess,dc=com', '-w', 'secret', '-b', 'dc=chess,dc=com']

    all_attribs = list(attribs.keys())

    allowed_attribs = list(set(all_attribs).intersection( set(person_attributes + organizationalPerson_attribs + inetOrgPerson_attribs)))
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

    p = ssh(host='challenge_container', user='chess', password='chess')

    sh = p.process(args)

    res = sh.recv()

    sh.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    print(res)

    return 0

def ls_or( host, port, base_dir, attribs):
    print('[TEST] ldapsearch(or test)')
    # ./ldapsearch -x -D "cn=admin,dc=chess,dc=com" -w secret -b "dc=chess,dc=com" objectClass=?

    args = [base_dir + 'ldapsearch', '-h', host, '-p', port, '-x', '-D', 'cn=admin,dc=chess,dc=com', '-w', 'secret', '-b', 'dc=chess,dc=com']

    all_attribs = list(attribs.keys())

    allowed_attribs = list(set(all_attribs).intersection( set(person_attributes + organizationalPerson_attribs + inetOrgPerson_attribs)))
    oc = random.choice([b'top', b'person', b'organizationalPerson', b'inetOrgPerson', b'*'])

    conditions = '(|(objectClass=' + oc.decode('UTF-8') + ')'

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

    p = ssh(host='challenge_container', user='chess', password='chess')

    sh = p.process(args)

    res = sh.recv()

    sh.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    print(res)

    return 0

def ls_ast( base_dir, attribs):
    pass

def ls_base( host, port, base_dir, attribs):
    print('[TEST] ldapsearch(base case)')
    args = [base_dir + 'ldapsearch', '-h', host, '-p', port, '-x', '-D', 'cn=admin,dc=chess,dc=com', '-w', 'secret', '-b', 'dc=chess,dc=com']

    p = ssh(host='challenge_container', user='chess', password='chess')

    sh = p.process(args)

    res = sh.recv()

    sh.close()
    p.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    print(res)

    return 0

def ldapsearch( host, port, base_dir, yolo_attribs, hj_attribs ):

    # ./ldapsearch -x -D "cn=admin,dc=chess,dc=com" -w secret -b "dc=chess,dc=com"

    ## limit by objectClass
    ## just display certain fields
    ## & operator
    ## | operator
    ## ! operator
    ## asterix

    sel = random.choice( [ yolo_attribs, hj_attribs] )

    ### modify existing
    ### add / delete entry
    ### add / increment / delete

    f = random.choice([ ls_or, ls_and, ls_cf, ls_oc, ls_base])

    f( host, port, base_dir, sel )

    return 0


def lm_mod( host, port, base_dir, attribs ):
    print('[TEST] ldapmodify (replace)')
    # select an attribute to modify
    a = ''

    ## get a list of the attributes
    all_attribs = list(attribs.keys())

    ## I only want the set of those attributes that are already associatied with the entry along with those I am willing to modify
    ### as specified in the global attribute lists
    allowed_attribs = set(all_attribs).intersection( set(person_attributes + organizationalPerson_attribs + inetOrgPerson_attribs))

    ## Randomly select one of the valid attributes
    selected_attrib = random.choice( list(allowed_attribs) )

    ## Save the orginal value
    original_value = attribs[selected_attrib]

    new_value = gen_random_string( random.randint( 5, 15) )

    print('\t[INFO] Changing %s from %s to %s' %(selected_attrib, original_value, new_value))

    ## Generate the new ldif data
    new_ldif = genrep( attribs[b'dn'], selected_attrib, new_value )

    ## Write the new ldif data to a file. I hard coded /tmp/nl.dif
    write_ldif( new_ldif )

    args = [base_dir + 'ldapmodify', '-h', host, '-p', port, '-x', '-D', 'cn=admin,dc=chess,dc=com', '-w', 'secret', '-f', '/tmp/nl.ldif'  ]

    p = ssh(host='challenge_container', user='chess', password='chess')

    sh = p.process(args)

    res = sh.recv()

    sh.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    print('\t[INFO] Returning %s from %s to %s' %(selected_attrib, new_value, original_value))
    ## Now we need to return the entry to its original value to maintain testing continuity
    orig_ldif = genrep( attribs[b'dn'], selected_attrib, original_value )    

    write_ldif( orig_ldif )

    sh = p.process(args)

    res = sh.recv()

    sh.close()
    p.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    return 0

def lm_add( host, port, base_dir, attribs ):
    print('[TEST] ldapmodify (add)')
    # select an attribute to modify
    a = ''

    ## get a list of the attributes
    all_attribs = list(attribs.keys())

    ## I want the set of attributes not already assigned to the person
    allowed_attribs = set( set(person_attributes + organizationalPerson_attribs + inetOrgPerson_attribs) - set(all_attribs))

    ## Randomly select one of the valid attributes
    selected_attrib = random.choice( list(allowed_attribs) )

    new_value = gen_random_string( random.randint( 5, 15) )

    print('\t[INFO] Adding attribute %s: %s' %(selected_attrib, new_value))

    ## Generate the new ldif data
    new_ldif = genadd( attribs[b'dn'], selected_attrib, new_value )

    ## Write the new ldif data to a file. I hard coded /tmp/nl.dif
    write_ldif( new_ldif )

    args = [base_dir + 'ldapmodify', '-h', host, '-p', port, '-x', '-D', 'cn=admin,dc=chess,dc=com', '-w', 'secret', '-f', '/tmp/nl.ldif'  ]

    p = ssh(host='challenge_container', user='chess', password='chess')

    sh = p.process(args)

    res = sh.recv()

    sh.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    print('\t[INFO] Removing attribute: %s' %(selected_attrib))
    ## Now we need to return the entry to its original value to maintain testing continuity
    orig_ldif = gendel( attribs[b'dn'], selected_attrib )    

    write_ldif( orig_ldif )
    
    sh = p.process(args)

    res = sh.recv()

    sh.close()
    p.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    return 0

def lm_inc( host, port, base_dir, attribs ):
    pass

## select either yolo or hj and modify their attributes
def ldapmodify( host, port, base_dir, yolo_attribs, hj_attribs ):
    sel = random.choice( [ yolo_attribs, hj_attribs] )

    ### modify existing
    ### add / delete entry
    ### add / increment / delete

    f = random.choice([ lm_add, lm_mod]) #, lm_add, lm_inc])

    f( host, port, base_dir, sel )

    return 0

def ldapwhoami( host, port, base_dir, yolo_attribs, hj_attribs):
    sel = random.choice( [ yolo_attribs, hj_attribs] )

    args = [base_dir + 'ldapwhoami', '-h', host, '-p', port, '-x', '-D', sel[b'dn'], '-w', base64.b64decode(sel[b'userPassword']) ]

    p = ssh(host='challenge_container', user='chess', password='chess')

    sh = p.process(args)

    res = sh.recv()

    sh.close()
    p.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    print(res)

    return 0

def ldapcompare( host, port, base_dir, yolo_attribs, hj_attribs):
    sel = random.choice( [ yolo_attribs, hj_attribs] )

    args = [base_dir + 'ldapcompare', '-h', host, '-p', port, '-x', '-D', 'cn=admin,dc=chess,dc=com', '-w', 'secret', sel[b'dn'] ]

    ## get a list of the attributes
    all_attribs = list(sel.keys())

    ## I only want the set of those attributes that are already associatied with the entry along with those I am willing to modify
    ### as specified in the global attribute lists
    allowed_attribs = set(all_attribs).intersection( set(person_attributes + organizationalPerson_attribs + inetOrgPerson_attribs))

    ## Randomly select one of the valid attributes
    selected_attrib = random.choice( list(allowed_attribs) )

    if random.randint( 0, 100) > 50:
        args.append( selected_attrib + b':' + sel[selected_attrib])
    else:
        args.append( selected_attrib + b':' + gen_random_string( 6) )

    p = ssh(host='challenge_container', user='chess', password='chess')

    sh = p.process(args)

    res = sh.recv()

    sh.close()
    p.close()

    if res.find(b'contact LDAP') != -1:
        print('[FAIL] Failed to connect to LDAP server')
        sys.exit(1)

    print(res)

    return 0

def run_test( host, port, path ):
    yolo_attribs = parse_ldif( './yolo.ldif' )

    hj_attribs = parse_ldif( './hj.ldif' )

    f = random.choice( [ldapcompare, ldapwhoami, ldapmodify, ldapsearch])
    return f( host, port, path, yolo_attribs, hj_attribs)

if __name__ == '__main__':
    if len(sys.argv) == 4:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
        LISTEN_PORT = int(sys.argv[3])
    else:
        if 'HOST' in os.environ and 'PORT' in os.environ and 'TOOLPATH':
            HOST = os.environ['HOST']
            PORT = os.environ['PORT']
            TOOLPATH = os.environ['TOOLPATH']
        else:
            print('[USAGE] %s <target_host> <target_port> <path_to_client_tools>' %(sys.argv[0]))
            print('[INFO] This poller acts as a launcher for the openldap client tools')
            print('       set of cornhill challenge binaries. It requires a running slapd')
            print('       instance the location of which is specified by target_port and')
            print('       target_host. The location of the client tools must also be')
            print('       specified via the path argument. You can also use the environment')
            print('       variables HOST, PORT, and TOOLPATH in lieu of command line args.')
            sys.exit(1)

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('[INFO] Poller for the Cornhill Challenge')
    print('[INFO] SEED: {0}'.format(SEED))

    random.seed( SEED )

    result = run_test(HOST, PORT, TOOLPATH)

    if result == 0:
        print('[SUCCESS] Poller was successfult')
        exit(0)
