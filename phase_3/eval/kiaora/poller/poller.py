import sys
import os
from pwn import *
import threading

import poller_config
import poller_threads



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
    print('Host ' + poller_config.HOST + ' and PORT: ' + poller_config.HOST, flush=True)

    threads = list()
    if poller_config.POLLER_MODE == 1:
        print("POLLER in automated tutorial mode with", poller_config.NUMBER_OF_THREADS, "users", flush=True)
    if poller_config.POLLER_MODE == 2:
        print("POLLER in automated player mode with", poller_config.NUMBER_OF_THREADS, "users", flush=True)
    if poller_config.POLLER_MODE == 3:
        print("POLLER in manual play mode, view README for how to use.", flush=True)
        poller_config.NUMBER_OF_THREADS=1

    for index in range(poller_config.NUMBER_OF_THREADS):
        print("Main: create and start thread ", index, flush=True)
        x = threading.Thread(target=poller_threads.thread_function, args=(index,))
        threads.append(x)
        x.start()

    for index, thread in enumerate(threads):
        thread.join()

    print("All threads completed", flush=True)
    for res in poller_config.USER_STATUS:
        #print(res, poller_config.USER_STATUS[res])
        if poller_config.USER_STATUS[res] == 1:
            print(res, "likely crashed")
            poller_config.POLLER_SUCCESS = False
    if poller_config.POLLER_SUCCESS == True:
        print("[SUCCESS] Poller Successful")
    else:
        print("[FAILED] Poller Failed")

if __name__ == "__main__":
    main()