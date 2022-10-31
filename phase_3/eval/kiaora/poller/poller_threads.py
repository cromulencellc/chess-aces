from pwn import *
import time
import threading

import poller

import poller_config
import poller_login
import poller_comms

import poller_dofunc
import poller_newchar
import poller_usercontrol

def thread_function(name):
    print("Thread", name, "starting", flush=True)
    if poller_config.POLLER_MODE == 1: ## automated tutorial
        poller_newchar.do_New_Character()
        return
    if poller_config.POLLER_MODE == 2: ## automated players
        username=poller_login.getRandomUser()
        if (poller_config.USER_STATUS[username] == 0):
            poller_config.USER_STATUS[username] = 1
            # things to do:           
            if poller_dofunc.doFailedLogin(username) == 0:
                print("Logged in with bad password")
                return
            if poller_dofunc.doSuccessfulLogin(username) == 0:
                print("Could not login")
                return
            if poller_dofunc.doAutomatedPlayer(username) == 0:
                print("Game play failure")
                return
            poller_config.USER_STATUS[username] = 2
            print("Thread", name, "Successful")
            return
        else:
            print(username +  " is already in use")
            print("Thread", name, "finishing")
            return
        return
    if poller_config.POLLER_MODE == 3: ## user controlled player
        username=poller_login.getRandomUser()
        if (poller_config.USER_STATUS[username] == 0):
            poller_config.USER_STATUS[username] = 1
            poller_usercontrol.do_Character_Control(username)
            exit(0)
        else:
            print(username +  " is already in use")
            print("Thread", name, "finishing")
            return
        return        

