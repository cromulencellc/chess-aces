from tracemalloc import stop
from pwn import *
import time
import threading
import sys

import poller_config
import poller_comms
import poller_login


def update_every_second(stop, conn):
    while not stop():
        time.sleep(1)
        if conn is not None:
            if stop():
                break
            response=poller_comms.recvdata(conn, 0.5)
            if response is not None:
                print(response, flush=True)

def do_Character_Control(username):
    conn=remote(poller_config.HOST, poller_config.PORT)
    if poller_login.do_Persistent_Login(conn, username, poller_login.getUserPassword(username))==False:
        poller_config.POLLER_SUCCESS=False
        return -1
    cmd = None
    stop_threads=False
    t = threading.Thread(target=update_every_second, args=(lambda : stop_threads, conn,))
    t.start()
    while True:    
        cmd = input (":> ")
        poller_comms.senddata(conn, cmd)
        if cmd.replace('\n', '').replace('\r', '') == 'quit' or cmd.replace('\n', '').replace('\r', '') == 'q':
            break
    print("Done playing with MUD")
    stop_threads=True
    time.sleep(3)
    t.join()
    conn.close()
    return