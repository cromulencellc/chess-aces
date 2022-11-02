import sys
import socket
import random
import time
import string
import base64
import os
import datetime
import string
import signal
import copy

import paho.mqtt.publish as publish
import paho.mqtt.subscribe as subscribe
import paho.mqtt.client as mqtt
import paho

HOST_g = 'localhost'

def handler( signum, frame):
    print('[FAIL] sigalrm fired Poller failed')
    exit(1)

def randomstring(l):
    q = b''

    for _ in range(l):
        q += random.choice(string.ascii_lowercase).encode('UTF-8')

    return q

def connect( ip, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (ip, port) )
    except:
        print('Failed to connect to %s:%d' %(ip, port))
        sys.exit(1)

    return s

def RU( s, u ):
    z = ''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1).decode('utf-8')
    except Exception as e:
        print('Failed to receive data')
        print(e)
        return None

    return z

def RL( s ):
    return RU( s, '\n')

def SW(s, data):
    s.sendall(data)

def RB(s):
    return RU(s, '>>> ')

## This dict will be ordered by topic name under which will be a list of expected messages
topics = {}

def on_connect(client, userdata, flags, rc):

    print("Connected with result code "+str(rc))

    return 0

def on_message(client, userdata, msg):
    global topics

    m = msg.payload.decode('utf-8')

    if type(msg.topic) == type(b''):
        t = msg.topic.decode('utf-8')
    else:
        t = msg.topic

    if t not in topics:
        print(topics)
        print(f'[ERROR] unknown topic {t}')
        exit(1)

    if m not in topics[t]:
        print(f'[ERROR] did not expect {t}:{m}')
        exit(1)

    topics[t].remove(m)

    print(f'[SUCCESS] received: {t}:{m}')

    return 0

def new_subscribe(client):
    print('[TEST] new_subscribe')

    subs = random.randint(1, 3)

    v = 'chess'
    for x in range(subs):
        nm = randomstring(5).decode('utf-8')

        v += '/' + nm
    
    topics[v] = []
    client.subscribe( (v, random.randint(0,2)) )

    print(f'\t[INFO] subscribe {v}')

    return 0

def auth_fail(client):
    print('[TEST] auth_fail')

    if len(topics) == 0:
        new_subscribe(client)

    t = random.choice(list(topics.keys()))

    message = randomstring( 10 ).decode('utf-8')

    u = randomstring(10).decode('utf-8')
    p = randomstring(10).decode('utf-8')

    try:
        publish.single(t, message, hostname=HOST_g, auth={'username':u, 'password':p} )
    except:
        print('\t[SUCCESS] authorization failed successfully')
        return 0

    return 1

def new_unsubscribe(client):
    print('[TEST] new_unsubscribe')

    if len(topics) == 0:
        new_subscribe(client)

    t = random.choice(list(topics.keys()))
    topics.pop(t)

    client.unsubscribe(t)
    print(f'\t[INFO] unsubscribed from {t}')

    return 0

def new_publish(client):
    print('[TEST] new_publish')
    ## randomly select a topic

    if len(topics) == 0:
        new_subscribe(client)

    t = random.choice(list(topics.keys()))

    message = randomstring( 10 ).decode('utf-8')

    topics[t].append(message)

    publish.single(t, message, hostname=HOST_g, auth={'username':'chess', 'password':'chess'} )

    print(f'\t[INFO] Sent {t}:{message}')

    return 0

def main():
    global HOST_g

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

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('[INFO] SEED: {0}'.format(SEED))

    random.seed( SEED )

    HOST_g = HOST

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(60)

    client = mqtt.Client(client_id="yolo")
    client.on_connect = on_connect
    client.on_message = on_message

    client.username_pw_set('chess', 'chess')

    client.connect(HOST, PORT, 60)

    client.loop_start()
    time.sleep(1)

    fs = [new_subscribe, new_publish, auth_fail, new_unsubscribe]

    for _ in range(10):
        d = random.choice( fs )

        if d( client ):
            print('FAILURE: ')
            exit(0)

        time.sleep(2)

    stop = 0

    while stop == 0:
        stop = 1
        for x in topics:
            if len(topics[x]) != 0:
                print(topics[x])
                time.sleep(1)
                stop = 0

    client.loop_stop()

    client.disconnect()
    

    print('[SUCCESS] Poller completed!')

if __name__ == '__main__':
    main()
