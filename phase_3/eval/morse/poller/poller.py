from re import T
import sys
import os
import signal
import time
from pwn import *
import requests

import poller_api
import poller_callFunc
import poller_config
import poller_utility

def handler(signum, frame):
    print('Process timeout.')
    print('[FAILED] POLLER Failed')
    exit(0)

def main():
    if len(sys.argv) == 3:
        poller_config.HOST = sys.argv[1]
        poller_config.PORT = int(sys.argv[2])
    else:
        if 'HOST' in os.environ and 'PORT' in os.environ:
            poller_config.HOST = os.environ['HOST']
            poller_config.PORT = int(os.environ['PORT'])
        else:
            print('[ERROR] HOST and PORT must be specified via arguments or variables.')
            sys.exit(1)
    print('Host ' + poller_config.HOST + ' and PORT: ' + str(poller_config.PORT), flush=True)
    poller_config.BASEADDR='http://{}:{}'.format(poller_config.HOST, poller_config.PORT)
    poller_config.headers['Host'] = poller_config.HOST + ':' + str(poller_config.PORT)
 
    signal.signal(signal.SIGALRM, handler)
    signal.alarm(60)

    poller_callFunc.doPing()
    poller_callFunc.doFailLogin()
    poller_callFunc.doSuccessfulLogin()
    poller_callFunc.getMaxNumberOfTopics()
    poller_callFunc.getMaxNumberOfPosts()
    numOfOps=poller_utility.getRandomNumber(10,20)
    print("Performing", str(numOfOps), "operations", flush=True)
    for x in range(0, numOfOps):
        time.sleep(1)
        poller_callFunc.doEverything()
    print('[SUCCESS] POLLER Succeeded', flush=True)

if __name__ == '__main__':
    main()
