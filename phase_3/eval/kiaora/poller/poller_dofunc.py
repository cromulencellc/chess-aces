from pwn import *
import time

import poller_config
import poller_comms
import poller_login
import poller_automated
import poller_utility

def doSuccessfulLogin(username):
    conn = remote(poller_config.HOST, poller_config.PORT)
    result = poller_login.existingUserLogin(conn, username, poller_login.getUserPassword(username))

    if not poller_login.isLoggedIn(result, username):
        conn.close()
        poller_config.USER_STATUS[username] = 0
        poller_config.POLLER_SUCCESS = False
        return 0
    poller_utility.printMsg(username +": Successfully Logged in", poller_config.STATUS)
    conn.close()
    return 1

def doFailedLogin(username):
    conn = remote(poller_config.HOST, poller_config.PORT)
    result = poller_login.failLogin(conn, username)
    if poller_login.isLoggedIn(result, username):
        conn.close()
        poller_config.USER_STATUS[username] = 0
        poller_config.POLLER_SUCCESS = False
        return 0
    poller_utility.printMsg(username +": Successfully failed to log in", poller_config.STATUS)
    conn.close()
    return 1

def doAutomatedPlayer(username):
    conn=remote(poller_config.HOST, poller_config.PORT)
    if poller_login.do_Persistent_Login(conn, username, poller_login.getUserPassword(username))==False:
        poller_config.POLLER_SUCCESS=False
        return 0

    for x in range (0,poller_config.DIRECTION_OPERATIONS):
        print(username, "operation:", x, flush=True)
        possible_directions = poller_automated.doDirections(conn)
        for y in range (0,poller_config.TIMESTOCHECKFORCREATURES):
            description=poller_automated.doLook(conn)
            if poller_automated.doAttackRoutine(conn, description) == 1:
                poller_automated.doStatusCheckRoutine(conn)

        possible_directions=poller_automated.doLimitDirections(conn, description)
        if possible_directions is None:
            poller_utility.printMsg("No where to go", poller_config.STATUS)
            conn.close()
            return 1
        message = "I can go ", possible_directions
        poller_utility.printMsg(message, poller_config.STATUS)
        if len(possible_directions) == 1:
            which_direction = possible_directions[0]
        else:
            which_direction = poller_automated.doPickRandomDir(possible_directions)
        poller_automated.doGoDirection(conn, which_direction)
        time.sleep(2)
    
    poller_comms.senddata(conn, 'save') # main menu
    poller_utility.printMsg(poller_comms.recvdata(conn), poller_config.RESPONSE)
    poller_comms.senddata(conn, 'quit')
    poller_utility.printMsg(conn.recv(), poller_config.RESPONSE)
    conn.close()
    message="Completed ", poller_config.DIRECTION_OPERATIONS, " room changes"
    poller_utility.printMsg(message, poller_config.STATUS)
    conn.close()
    return 1

