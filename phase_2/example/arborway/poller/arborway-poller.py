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
    print('[FAIL] sigalrm fired Poller failed')
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

def shellcode_test(s):

    print('[INFO] Shellcode Test')

    sc = random.choice(os.listdir('./unit_tests/') )

    f = open('./unit_tests/' + sc, 'rb')

    data = f.read()
    f.close()

    print('[INFO] Sending shellcode: %s' %(sc) )
    SW(s, b'4\n')
    print(RU(s, 'size: '))
    SW(s, str(len(data)).encode('utf-8') + b'\n')
    print(RL(s))
    SC(s, data)

    SW(s, b'c\n')

    while True:
        y = RL(s)
        y = y.rstrip('\n')
        print(y)

        if y.find('Complete') != -1 or y.find('instruction exception') != -1:
            break;


    s.close()

    print('[INFO] Poller completed successfully')

    exit(0)

def elf_test(s):
    print('[INFO] ELF test')

    try:
        f = open('vdso64.so', 'rb')
    except:
        print('[ERROR] failed to open vdso64 file')
        sys.exit(0)

    vdso = f.read()
    f.close()

    ## TODO: There needs to be some randomization here
    try:
        f = open('elf.bin', 'rb')
    except:
        print('[ERROR] failed to open elf file')
        sys.exit(0)

    elf = f.read()
    f.close()

    RB(s)


    print('[INFO] Sending elf')
    SW(s, b'1\n')
    print(RU(s, 'size: '))
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

    SW(s, b'3\n')

    RL(s)
    SW(s, b'unset pr\n')
    RL(s)
    SW(s, b'c\n')

    print('[INFO] Executing now...')

    while True:
        y = RL(s)
        y = y.rstrip('\n')

        if y.find('Complete') != -1:
            break;

    s.close()

    print('[SUCCESS] Poller completed successfully')

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
        s = connect( HOST, PORT )
    except:
        print('[ERROR] Failed to connect to: %s:%d' %(HOST, PORT))
        exit(1)

    d = random.choice( [shellcode_test, elf_test] )

    d( s )

if __name__ == '__main__':
    main()
