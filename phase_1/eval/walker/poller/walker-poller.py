import sys
import os
import random
import socket
import string
import time
import base64


mailboxes = ["INBOX", [
             ["INBOX.childone", 
                [ [ "INBOX.childone.mba", [] ], 
                  [ "INBOX.childone.mbb", [] ], 
                  [ "INBOX.childone.mbc", [] ] ] ], 
             ["INBOX.childtwo", 
                [ [ "INBOX.childtwo.mba", [] ], 
                  [ "INBOX.childtwo.mbb", [] ], 
                  [ "INBOX.childtwo.mbc", [] ] ] ], 
             ["INBOX.childthr", 
                [ [ "INBOX.childthr.mba", [] ], 
                  [ "INBOX.childthr.mbb", [] ], 
                  [ "INBOX.childthr.mbc", [] ] ] ] ] ]

valid_parents = ["INBOX", "INBOX.childone", "INBOX.childtwo", "INBOX.childthr"]
valid_full = ["INBOX", "INBOX.childone", "INBOX.childone.mba", "INBOX.childone.mbb",
              "INBOX.childone.mbc", "INBOX.childtwo", "INBOX.childtwo.mba", 
              "INBOX.childtwo.mbb", "INBOX.childtwo.mbc", "INBOX.childthr", 
              "INBOX.childthr.mba", "INBOX.childthr.mbb", "INBOX.childthr.mbc" ]

def connect( ip, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (ip, port) )
    except:
        print('Failed to connect to %s:%d' %(ip, port))
        sys.exit(1)

    return s

def readuntil( s, u ):
    z = ''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1).decode('utf-8')
    except Exception as e:
        print('Failed to receive data')
        print(e)
        return None

    return z

def randomstring(l):
    q = ''

    for _ in range(l):
        q += random.choice(string.ascii_lowercase)

    return q

def sw(fd, line):
        fd.send(line.encode('utf-8'))

def uselogin( fd, user, pw):
    print("[TEST] Connecting with LOGIN")

    uid = randomstring(5)

    sw(fd,  uid + " LOGIN " + user + " " + pw + "\r\n")

    y = readuntil(fd, uid + ' OK LOGIN Ok.')
    
    print('[SUCCESS] LOGIN')
    return 1

def useauthenticate( fd, user, pw):
    print("[TEST] Connecting with AUTHENTICATE")

    uid = randomstring(5)

    sw(fd, uid + ' authenticate plain\n')

    y = readuntil(fd, '+\r\n')

    bd = '\x00' + user + '\x00' + pw

    enc = base64.b64encode(bd.encode('utf-8')).decode('utf-8') + '\r\n'

    sw(fd,  enc )

    y = readuntil(fd, uid + ' OK LOGIN Ok.')

    print('[SUCCESS] AUTHENTICATE')
    return 1

def login( fd, user, pw ):
    authfunc = random.choice( [uselogin, useauthenticate] )

    result = authfunc( fd, user, pw)

    return result

def handlecap( fd ):
    print("[TEST] CAPABILITY")

    uid = randomstring(5)

    sw(fd, uid + ' CAPABILITY\n')

    y = readuntil(fd, '* CAPABILITY IMAP4rev1 AUTH=PLAIN\r\n')

    y = readuntil(fd, uid + " OK CAPABILITY completed\r\n")

    print("[SUCCESS] CAPABILITY")

    return 1

def handlenoop(fd):
    print("[TEST] NOOP")

    uid = randomstring(5)

    sw(fd, uid + ' NOOP\n')

    y = readuntil( fd, uid + " OK NOOP completed\r\n")

    print("[SUCCESS] NOOP")

    return 1

def handlelogout(fd):
    print("[TEST] LOGOUT")

    uid = randomstring(5)

    sw(fd, uid + ' LOGOUT\n')

    y = readuntil(fd, "* BYE CHESS-IMAP server shutting down\r\n")

    y = readuntil( fd, uid + " OK LOGOUT completed\r\n")

    print("[SUCCESS] LOGOUT")

    return 1

def listall(fd):
    print("[TEST] listall")

    uid = randomstring(5)

    sw(fd, uid + ' LIST "" *\n')

    y = readuntil(fd, uid + " OK LIST completed\r\n")

    print('[SUCCESS] listall')
    return 1

def list_parent_subset( fd ):
    print("[TEST] list_parent_subset")

    uid = randomstring(5)

    ## Select a parent
    pt = random.choice(valid_parents)

    ## Get a subsection of the parent
    start = random.randint(0, int(len(pt)/2))
    end = random.randint(start+1, len(pt))

    ss = pt[start:end]
    print("\tsubstr: " + ss)

    cmd = uid + ' LIST "%s" *\n' %(ss)
    sw(fd, cmd)

    y = readuntil(fd, uid + " OK LIST completed\r\n")

    print("[SUCCESS] list_parent_subset")
    return 1

def list_child_subset( fd ):
    print("[TEST] list_child_subset")

    uid = randomstring(5)

    ## Select a parent
    pt = random.choice(valid_parents)

    ## Get a subsection of the parent
    start = random.randint(0, int(len(pt)/2))
    end = random.randint(start+1, len(pt))

    ss = pt[start:end]
    print("\tsubstr: " + ss)

    cmd = uid + ' LIST "" "%s"\n' %(ss)
    sw(fd, cmd)

    y = readuntil(fd, uid + " OK LIST completed\r\n")

    print("[SUCCESS] list_child_subset")
    return 1

def create_delete( fd ):
    print("[TEST] create_delete")

    newone = randomstring( 6 )
    uid = randomstring(5)

    pt = random.choice( valid_parents )

    full = pt + '.' + newone

    sw(fd, uid + ' CREATE ' + full + '\n')

    y = readuntil( fd, uid + ' OK "%s" created.\r\n' %full)

    sw(fd, uid + ' DELETE ' + full + '\n')

    y = readuntil( fd, uid + ' OK Folder deleted.\r\n')

    print('[SUCCESS] create_delete')
    return 1


def handlestatus(fd):
    print("[TEST] handlestatus")

    uid = randomstring(5)

    pt = random.choice( valid_parents )

    st = [ 'MESSAGES', 'RECENT', 'UNSEEN', 'PASSED', 'REPLIED', 'SEEN', 'TRASHED', 'DRAFT', 'FLAGGED' ]

    sel = random.sample(range(len(st)), random.randint(1, len(st)))

    cmd = uid + ' STATUS ' + pt + ' ('

    for x in sel:
        cmd += st[x] + ' '

    cmd += ')\n'

    sw(fd, cmd)

    y = readuntil( fd, uid + ' OK STATUS completed\r\n')

    print('[SUCCESS] handlestatus')
    return 1


def handlecreate_rename_delete(fd):
    print("[TEST] handlecreate_rename_delete")

    newone = randomstring( 6 )
    copyto = randomstring( 6 )

    uid = randomstring(5)

    pt = random.choice( valid_parents )

    full = pt + '.' + newone

    copyto_full = pt + '.' + copyto

    cmd = uid + ' CREATE ' + full + '\n'
    sw(fd, uid + ' CREATE ' + full + '\n')

    y = readuntil( fd, uid + ' OK "%s" created.\r\n' %full)

    cmd = uid + ' RENAME ' + full + ' ' + copyto_full + '\n'
    print(cmd)

    sw(fd, uid + ' RENAME ' + full + ' ' + copyto_full + '\n' )

    y = readuntil( fd, uid + ' OK Folder renamed.\r\n')

    sw(fd, uid + ' DELETE ' + copyto_full + '\n')

    y = readuntil( fd, uid + ' OK Folder deleted.\r\n')

    print('[SUCCESS] handlecreate_rename_delete')
    return 1

def select_store_close(fd):
    print("[TEST] select_store_close")

    uid = randomstring(5)

    pt = random.choice( valid_parents )

    sw(fd, uid + ' SELECT ' + pt + '\n')

    y = readuntil(fd, uid + " OK [READ-WRITE] Ok\r\n")

    if ( random.randint(0,100) > 50):
        r = random.choice(['1', '2'])
    else:
        r = random.choice(['1', '2']) + ':' + random.choice(['2', '3'])

    flags = ['\\Seen', '\\Answered', '\\Flagged', '\\Deleted', '\\Draft']

    sel = random.sample(range(len(flags)), random.randint(1, len(flags)))

    cmd = uid + ' store ' + r + ' ' + random.choice(['FLAGS', '+FLAGS', '-FLAGS']) + ' ('

    for x in sel:
        cmd += flags[x] + ' '

    cmd = cmd.rstrip() + ')\n'

    print(cmd)

    sw(fd, cmd)

    y = readuntil( fd, uid + " OK STORE completed\r\n")

    sw(fd, uid + ' CLOSE\n')

    y = readuntil(fd, uid + " OK CLOSE completed\r\n")

    print('[SUCCESS] select_store_close')
    return 1

def handleselect_copy_close(fd):
    print("[TEST] handleselect_copy_delete")

    uid = randomstring(5)

    pt = random.choice( valid_parents )

    sw(fd, uid + ' SELECT ' + pt + '\n')

    y = readuntil(fd, uid + " OK [READ-WRITE] Ok\r\n")

    if ( random.randint(0,100) > 50):
        r = random.choice(['1', '2'])
    else:
        r = random.choice(['1', '2']) + ':' + random.choice(['2', '3'])


    cmd = uid + ' COPY ' + r + ' ' + random.choice(valid_full) + '\n'

    sw( fd, cmd)

    y = readuntil( fd, uid + " OK COPY completed.\r\n")

    print('[SUCCESS] handleselect_copy_delete')
    return 1

def select_expunge_close(fd):
    print("[TEST] select_expunge_close")

    uid = randomstring(5)

    pt = random.choice( valid_parents )

    sw(fd, uid + ' SELECT ' + pt + '\n')

    y = readuntil(fd, uid + " OK [READ-WRITE] Ok\r\n")

    if ( random.randint(0,100) > 50):
        r = random.choice(['1', '2'])
    else:
        r = random.choice(['1', '2']) + ':' + random.choice(['2', '3'])

    cmd = uid + ' store ' + r + ' +FLAGS (\\Deleted)\n'

    sw(fd, cmd)

    y = readuntil( fd, uid + " OK STORE completed\r\n")

    sw( fd, uid + ' EXPUNGE\n')

    y = readuntil( fd, uid + " OK EXPUNGE completed\r\n")

    sw(fd, uid + ' CLOSE\n')

    y = readuntil(fd, uid + " OK CLOSE completed\r\n")

    print('[SUCCESS] select_expunge_close')
    return 1

## [ full | flags | internaldate | rfc822 | rfc822.size | rfc822.header | rfc822.text | envelope | body | body[<headers>]
def select_fetch(fd):
    print("[TEST] select_fetch")

    uid = randomstring(5)

    pt = random.choice( valid_parents )

    sw(fd, uid + ' SELECT ' + pt + '\n')

    y = readuntil(fd, uid + " OK [READ-WRITE] Ok\r\n")

    if ( random.randint(0,100) > 50):
        r = random.choice(['1', '2'])
    else:
        r = random.choice(['1', '2']) + ':' + random.choice(['2', '3'])

    cmd = uid + ' FETCH ' + r + ' ('

    attribs = [ "full", "flags", "internaldate", "rfc822", "rfc822.size", "rfc822.header", "rfc822.text", "envelope", "body"]
    headers = ["From", "To", "Date", "Subject"]

    if ( random.randint(0,100) > 90):
        sel = random.sample(range(len(headers)), random.randint(1, (int(len(headers)/2))+1))
        
        cmd += 'BODY['

        for x in sel:
            cmd += headers[x] + ' '

        cmd = cmd.rstrip() + ']'
    else:
        sel = random.sample(range(len(attribs)), random.randint(1, (int(len(headers)/2))+1))

        for x in sel:
            cmd += attribs[x] + ' '

    cmd = cmd.rstrip() + ')\n'

    sw(fd, cmd )

    y = readuntil(fd, uid + " OK FETCH completed.\r\n")

    sw(fd, uid + ' CLOSE\n')

    y = readuntil(fd, uid + " OK CLOSE completed\r\n")

    print('[SUCCESS] select_fetch')
    return 1


def run_test( HOST, PORT ):
    server_fd = connect( HOST, PORT )

    login( server_fd, 'chess', 'chess')

    allofem = [handlecap, handlenoop, listall, list_parent_subset, list_child_subset, create_delete, handlestatus, handlecreate_rename_delete, select_store_close, handleselect_copy_close, select_expunge_close, select_fetch]
    now = [handlecap]

    for _ in range(2, 10):
        z = random.choice( allofem )

        z( server_fd )

    server_fd.close()

    print('[SUCCESS] All tests succeeded')
    sys.exit()

def main():

    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
    else:
        if 'HOST' in os.environ and 'PORT' in os.environ:
            HOST = os.environ['HOST']
            PORT = int(os.environ['PORT'])
        else:
            print('[ERROR] target and port must be specified via arguments or variables.')
            sys.exit(1)

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('[INFO] Poller for the Walker Challenge Binary')
    print('[INFO] SEED: {0}'.format(SEED))

    random.seed( SEED )

    run_test(HOST, PORT)
    print('[SUCCESS] Poller completed successfully')

if __name__ == '__main__':
    main()