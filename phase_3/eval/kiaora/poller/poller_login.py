from pwn import *
import re
import random

import poller_config
import poller_cheatsheet
import poller_comms

def getRandomUser():
    usernum=random.randint(0, poller_config.maxusers-1)
    return poller_config.users[usernum]

def getUserPassword(user):
    return poller_cheatsheet.passwords[user]

def existingUserLogin(conn, username, password):
    conn.recvuntil(b'By what name do they call you? ').decode('utf-8')
    poller_comms.senddata(conn, username)
    result=poller_comms.recvdata(conn)
    ## check for message for "ATTENTION" means bad password previously
    if isBadPassword(result):
        print("Previous bad password")
        poller_comms.senddata(conn, '\n')
        poller_comms.recvdata(conn)
    poller_comms.senddata(conn, password, 0)
    result = conn.recv()
    if result == poller_config.header:
        return conn.recv()
    else:
        return result

def failLogin(conn, username):
    conn.recvuntil(b'By what name do they call you? ').decode('utf-8') # )
    poller_comms.senddata(conn, username)
    result=poller_comms.recvdata(conn)
    if isBadPassword(result): ## check for message for "ATTENTION" means bad password previously
        print("Previous bad password")
        poller_comms.senddata(conn, '\n')
        poller_comms.recvdata(conn)
    poller_comms.senddata(conn, poller_config.badpassword)
    result = conn.recv()
    if result == poller_config.header:
        return conn.recv()
    else:
        return result

def isBadPassword(message):
    findAttention=message.find('ATTENTION')
    if findAttention == -1:
        return False
    return True

def isLoggedIn(message, username):
    footerfound = False
    if message.find(poller_config.header) >= 0:
        message = message[message.find(poller_config.header) + 5:]
    if message.find(poller_config.footer) >= 0:
        footerfound = True
        message = message[:message.find(poller_config.footer)]
    if message.find(poller_config.go_ahead) >= 0 and not footerfound:
        message = message[:message.find(poller_config.go_ahead)]
    
    message=message.decode('utf-8').replace('\n', ' ').replace('\r', '')
    username = re.sub('([a-zA-Z])', lambda x: x.groups()[0].upper(), username, 1)
    welcomemsg='Welcome back ' + username + ', you last visited DikuMUD III Git Vanilla'
    findWelcome=message.find(welcomemsg)
    if findWelcome == -1:
        return False
    return True

def do_Persistent_Login(conn, username, password):
    result = existingUserLogin(conn, username, password)
    if not isLoggedIn(result, username):
        conn.close()
        print(username + ': Unable to Login')
        return False
    print(username +": Logged in")
    return True
