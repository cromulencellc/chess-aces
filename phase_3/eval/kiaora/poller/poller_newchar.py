from pwn import *
import time

import poller_config
import poller_comms
import poller_login
import poller_utility

def newuser_login(username, password):
    conn = remote(poller_config.HOST, poller_config.PORT)
    yes = 'Y'
    ## NEW USER
    poller_utility.printMsg(conn.recvuntil('By what name do they call you? ').decode('utf-8'), poller_config.RESPONSE)
    poller_comms.senddata(conn, username)
    poller_utility.printMsg(poller_comms.recvdata(conn), poller_config.RESPONSE)
    poller_comms.senddata(conn, yes)
    poller_utility.printMsg(poller_comms.recvdata(conn), poller_config.RESPONSE)
    poller_comms.senddata(conn, password)
    poller_utility.printMsg(poller_comms.recvdata(conn), poller_config.RESPONSE)
    poller_comms.senddata(conn, password)
    poller_utility.printMsg(poller_comms.recvdata(conn), poller_config.RESPONSE)
    poller_comms.senddata(conn, str(poller_utility.getRandomNumber(1,2))) # female or male (1-2)
    poller_utility.printMsg(poller_comms.recvdata(conn), poller_config.RESPONSE)
    race=str(poller_utility.getRandomNumber(1,11))
    poller_comms.senddata(conn, race) # race (1-11)
    poller_utility.printMsg(poller_comms.recvdata(conn), poller_config.RESPONSE)
    profession=str(poller_utility.getRandomNumber(1,4))
    poller_comms.senddata(conn, '1') # profession (1-4) 3=saint
    poller_utility.printMsg(poller_comms.recvdata(conn), poller_config.RESPONSE)

    personality=str(poller_utility.getRandomNumber(1,3))
    poller_comms.senddata(conn, '1') # personality (1-3) (1-2saint)
    poller_utility.printMsg(poller_comms.recvdata(conn), poller_config.RESPONSE)
  
    ##MAIN MENU
    poller_comms.senddata(conn, '3') # main menu (1-4) 3=join game
    poller_utility.printMsg(poller_comms.recvdata(conn), poller_config.RESPONSE)
    poller_comms.senddata(conn, 'save') # main menu
    poller_utility.printMsg(poller_comms.recvdata(conn), poller_config.RESPONSE)
    poller_comms.senddata(conn, 'quit')
    poller_utility.printMsg(conn.recv(), poller_config.RESPONSE)
    conn.close()

def retriveMessages(conn, untilmsg):
    tries=10
    while tries > 0:
        time.sleep(1)
        response=poller_comms.recvdata(conn)
        if response is not None:
            tries = 10
            response=response.replace('\n', ' ').replace('\r', '')
            poller_utility.printMsg(response, poller_config.RESPONSE)
            found=poller_utility.findinmessage(response, untilmsg)
            if found != -1:
                return found
        tries-=1
    return -1

def retriveMessagesUntilType(conn):
    tries=10
    while tries > 0:
        time.sleep(1)
        response=poller_comms.recvdata(conn)
        if response is not None:
            tries = 10
            response=response.replace('\n', ' ').replace('\r', '')
            poller_utility.printMsg(response, poller_config.RESPONSE)
            if poller_utility.findinmessage(response, 'just \'nod\'') != -1:
                return 'nod'
            if poller_utility.findinmessage(response, 'the newbie guide waves goodbye.') != -1:
                return 'save'
            found=poller_utility.findinmessage(response, 'typ')        
            if found != -1:
                typeLoc=poller_utility.findinmessage(response[found:], 'type:')
                if typeLoc == -1:
                    typeLoc=poller_utility.findinmessage(response[found:], 'type')
                if typeLoc == -1:
                    typeLoc=poller_utility.findinmessage(response[found:], 'typing')
                if typeLoc != -1:
                    typeLoc+=found 
                    firstquote=typeLoc+poller_utility.findinmessage(response[typeLoc:], '\'')
                    if firstquote > typeLoc + 25:
                        continue
                    secondquote=firstquote+poller_utility.findinmessage(response[firstquote+1:],'\'')
                    if firstquote != -1 and secondquote != -1:
                        print("USE MESSAGE:", response[firstquote+1:secondquote+1], flush=True)
                        ##need to have checks for 
                        return response[firstquote+1:secondquote+1]
        tries-=1
    if tries == 0:
        print("Ran out of tries", flush=True)
        return 0
    print("Something else")
    return 0  

def doMessageRoutine(conn, message):
    time.sleep(1)
    response=poller_comms.recvdata(conn)
    if response is not None:
        response=response.replace('\n', ' ').replace('\r', '')
        poller_utility.printMsg(response, poller_config.RESPONSE)
    poller_comms.senddata(conn, message) 
    return retriveMessagesUntilType(conn)

def doNewbieGuide(conn):
    poller_comms.sendSingleMessage(conn, "read handbook")
    poller_comms.sendSingleMessage(conn, "read chapter 1")
    print("Following Newbie Guide")
    poller_comms.sendSingleMessage(conn, 'say help me newbie guide')
    if retriveMessages(conn, 'say tour for the quest') == 0:
        print("Failed to find tour message")
        exit(0)
    result='say tour'
    while result != 0:
        msg=result
        result=doMessageRoutine(conn, msg)
        if result == 'write letter':
            poller_comms.sendSingleMessage(conn, 'write letter')
            result=retriveMessages(conn, ']')
            if result == -1:
                print("Couldn't find next message")
                exit(0)
            poller_comms.sendSingleMessage(conn, 'hi')
            time.sleep(1)
            poller_comms.sendSingleMessage(conn, '@')
            time.sleep(1)
            result = 'say DONE'
        if result == 'save':
            poller_comms.sendSingleMessage(conn, 'save')
            time.sleep(1)
            poller_comms.sendSingleMessage(conn, 'quit')
            return 1
    print("Couldn't find next message")
    return -1

def do_New_Character():
    print("Creating new user")
    username=poller_utility.getRandomString(0,8)
    password=poller_utility.getRandomString(1,8)
    print("User:", username)
    print("Pass:", password)
    #conn=remote(poller_config.HOST, poller_config.PORT)
    #verify
    print("Logging into new user")
    newuser_login(username, password)
    time.sleep(3)
    print("Logging into user", flush=True)
    conn=remote(poller_config.HOST, poller_config.PORT)
    if poller_login.do_Persistent_Login(conn, username, password)==False:
        poller_config.POLLER_SUCCESS=False
        return
    if doNewbieGuide(conn) == 1:
        print("[SUCCESS]Newbie Guide Success")
    else:
        print("[FAILURE]Newbie Guide Failed")
    conn.close()
