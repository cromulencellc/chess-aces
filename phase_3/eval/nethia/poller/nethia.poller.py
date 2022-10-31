import sys
import os
import random
import socket
import string
import time
from pwn import *

def connect( ip, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (ip, port) )
    except:
        print('Failed to connect to %s:%d' %(ip, port))
        sys.exit(1)

    return s  

class ircuser:
    def __init__(self, HOST, PORT):
        self.fd = remote( HOST, PORT)

        self.nick = randomstring(8).encode('utf-8')
        self.user = randomstring(8).encode('utf-8')
        self.realname = randomstring(8).encode('utf-8')

        self.channels = {}

        self.knocksent = 0

    def getaline(self):
        l = self.fd.readuntil( b'\r\n').rstrip()

        self.lastline = l

        return l


    def getline( self, expected ):
        l = self.fd.readuntil(b'\r\n').rstrip()
        self.lastline = l

        if l.find(expected) == -1:
            print('[FAIL] received: {}'.format(l))
            print('[FAIL] expected: {}'.format(expected))
            return 0

        return 1

    def loginthing( self ):
        self.fd.send(b'CAP LS\r\n')
        self.fd.readuntil(b'\r\n')
        self.fd.send(b'NICK %s\r\n' %(self.nick))
        self.fd.send(b'USER %s %s  localhost :%s\r\n' %(self.user, self.user, self.user))
        self.fd.send(b'CAP REQ :multi-prefix\r\n')

        y = self.fd.readuntil(b'\r\n')

        if y.startswith(b'PING') == True:
            z = y.split(b':')[1]
            self.fd.send(b'PONG :' + z + b'\r\n')

        self.fd.send(b'CAP END\r\n')
        self.fd.readuntil(b'+ix\r\n')

        return 1

    def join_new_chan(self):
        cn = b'#' + randomstring( random.randint(5,10)).encode('utf-8')

        d = b'JOIN ' + cn + b'\r\n'

        self.sw(d)

        y = self.fd.readuntil(b'\r\n').rstrip().split(b' ')

        if len(y) < 3:
            print('[FAIL] invalid response length: {}'.format(y))
            return 0

        if y[1] != b'JOIN':
            print('[FAIL] invalid JOIN response: {}'.format(y))
            return 0

        if y[2] != (b':' + cn):
            print('[FAIL] invalid channel name response: {}'.format(y))
            return 0

        y = self.fd.readuntil(b'\r\n').rstrip().split(b' ')

        if len(y) < 4:
            print('[FAIL] invalid MODE response length: {}'.format(y))
            return 0

        if y[1] != b'MODE':
            print('[FAIL] invalid MODE response: {}'.format(y))
            return 0

        if y[2] != (cn):
            print('[FAIL] invalid channel name response: {}'.format(y))
            return 0

        if y[3] != (b'+nt'):
            print('[FAIL] invalid channel name response: {}'.format(y))
            return 0

        y = self.fd.readuntil(b'\r\n').rstrip().split(b' ')

        if len(y) < 6:
            print('[FAIL] invalid response length: {}'.format(y))
            return 0

        if y[1] != b'353':
            print('[FAIL] invalid 353 response: {}'.format(y))
            return 0

        if y[2] != self.nick:
            print('[FAIL] invalid nick response: {}'.format(y))
            return 0

        if y[4] != (cn):
            print('[FAIL] invalid channel name response: {}'.format(y))
            return 0

        y = self.fd.readuntil(b'\r\n').rstrip().split(b' ')

        if len(y) < 5:
            print('[FAIL] invalid response length: {}'.format(y))
            return 0

        if y[1] != b'366':
            print('[FAIL] invalid 366 response: {}'.format(y))
            return 0            

        self.channels[cn] = b'+tn'
        print('[INFO] {} Joined {}'.format(self.nick, cn))

        return cn

    def do_some_joins( self ):
        for i in range( random.randint(2,5)):
            cn = b'#' + randomstring( random.randint(5,10)).encode('utf-8')

            d = b'JOIN ' + cn + b'\r\n'

            self.sw(d)

            y = self.fd.readuntil(b'\r\n').rstrip().split(b' ')

            if len(y) < 3:
                print('[FAIL] invalid response length: {}'.format(y))
                return 0

            if y[1] != b'JOIN':
                print('[FAIL] invalid JOIN response: {}'.format(y))
                return 0

            if y[2] != (b':' + cn):
                print('[FAIL] invalid channel name response: {}'.format(y))
                return 0

            y = self.fd.readuntil(b'\r\n').rstrip().split(b' ')

            if len(y) < 4:
                print('[FAIL] invalid MODE response length: {}'.format(y))
                return 0

            if y[1] != b'MODE':
                print('[FAIL] invalid MODE response: {}'.format(y))
                return 0

            if y[2] != (cn):
                print('[FAIL] invalid channel name response: {}'.format(y))
                return 0

            if y[3] != (b'+nt'):
                print('[FAIL] invalid channel name response: {}'.format(y))
                return 0

            y = self.fd.readuntil(b'\r\n').rstrip().split(b' ')

            if len(y) < 6:
                print('[FAIL] invalid response length: {}'.format(y))
                return 0

            if y[1] != b'353':
                print('[FAIL] invalid 353 response: {}'.format(y))
                return 0

            if y[2] != self.nick:
                print('[FAIL] invalid nick response: {}'.format(y))
                return 0

            if y[4] != (cn):
                print('[FAIL] invalid channel name response: {}'.format(y))
                return 0

            y = self.fd.readuntil(b'\r\n').rstrip().split(b' ')

            if len(y) < 5:
                print('[FAIL] invalid response length: {}'.format(y))
                return 0

            if y[1] != b'366':
                print('[FAIL] invalid 366 response: {}'.format(y))
                return 0            

            self.channels[cn] = b'+tn'
            print('[INFO] {} Joined {}'.format(self.nick, cn))

        print ('[INFO] Channel joins successful')

        return 1

    def sw( self, line):
        self.fd.send(line)

## This dictionary will hold the channel name and any set modes
channels = {}

def randomstring(l):
    q = ''

    for _ in range(l):
        q += random.choice(string.ascii_lowercase)

    return q

def set_invite(aaaa, bbbb):
    print('[TEST] set_invite')

    cn = aaaa.join_new_chan()

    ## Set the mode
    d = b'MODE ' + cn + b' +i\r\n'

    aaaa.sw(d)

    ## :aaaa!~aaa@aaa MODE #weoiru +i
    if ( aaaa.getline( b'MODE ' + cn + b' +i') == 0 ):
        print('[FAIL] invalid MODE +i response')
        return 0

    d = b'JOIN ' + cn + b'\r\n'

    bbbb.sw( d )

    ###:irc.ubuntu-bionic 473 aaaa #bbbb :Cannot join channel (+i) - you must be invited
    if ( bbbb.getline( b'473 ' + bbbb.nick + b' ' + cn + b' :Cannot join' ) == 0):
        print('[FAIL] invalid 473 join fail response')
        return 0

    ### Send invite
    d = b'INVITE ' + bbbb.nick + b' ' + cn + b'\r\n'
    aaaa.sw(d)

    ## :irc.ubuntu-bionic 341 aaaa bbbb #weoiru
    if ( aaaa.getline( b'341 ' + aaaa.nick + b' ' + bbbb.nick + b' ' + cn) == 0 ):
        print('[FAIL] invalid invite request response')
        return 0

    y = aaaa.fd.readline().split(b' ')

    if ( y[1] != b'NOTICE' or y[2] != b'@'+cn or y[3] != b':' + aaaa.nick or y[5] != bbbb.nick ):
        print('[FAIL] received: ', y)
        return 0

    d = b'JOIN ' + cn + b'\r\n'

    bbbb.sw( d )

    if ( aaaa.getline( b'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid aaaa JOIN response')
        return 0

    y = bbbb.fd.readline().split(b' ')

    if ( y[1] != b'INVITE' or y[2] != bbbb.nick):
        print('[FAIL] bad invite: ', y)
        return 0

    if ( bbbb.getline( b'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid bbbb JOIN response')
        return 0

    if ( bbbb.getline( b'353 ' + bbbb.nick + b' = ' + cn) == 0 ):
        print('[FAIL] invalid JOIN 353 response')
        return 0

    if ( bbbb.getline( b'366 ' + bbbb.nick + b' ' + cn) == 0 ):
        print('[FAIL] invalid JOIN 366 response')
        return 0

    msg = randomstring( random.randint(5,15)).encode('utf-8')

    d = b'PART ' + cn + b' :' + msg + b'\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( b'PART ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid bbbb PART response')
        return 0

    if ( aaaa.getline( b'PART ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PART response')
        return 0

    d = b'MODE ' + cn + b' -i\r\n'

    aaaa.sw(d)

    if ( aaaa.getline( b'MODE ' + cn + b' -i') == 0 ):
        print('[FAIL] invalid MODE -i response')
        return 0

    msg = randomstring( random.randint(5,15)).encode('utf-8')
    d = b'PART ' + cn + b' :' + msg + b'\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( b'PART ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PART response')
        return 0

    aaaa.channels.pop(cn)

    print('[SUCCESS] set_invite')

    return 1

def set_key(aaaa, bbbb):
    print('[TEST] set_key')

    msg = randomstring( random.randint(10,20)).encode('utf-8')
    key = randomstring( random.randint(6,10)).encode('utf-8')

    cn = aaaa.join_new_chan()

    print(b'[INFO] Joined ' + cn)

    ## Set the mode
    d = b'MODE ' + cn + b' +k ' + key + b'\r\n'

    aaaa.sw(d)

    ## :aaaa!~aaa@aaa MODE #weoiru +i
    if ( aaaa.getline( b'MODE ' + cn + b' +k ') == 0 ):
        print('[FAIL] invalid MODE +k response')
        return 0

    d = b'JOIN ' + cn + b'\r\n'

    bbbb.sw( d )

    if ( bbbb.getline( b'475 ' + bbbb.nick + b' ' + cn + b' :Cannot join' ) == 0):
        print('[FAIL] invalid 473 join fail response')
        return 0

    d = b'JOIN ' + cn + b' ' + key + b'\r\n'

    bbbb.sw( d )

    if ( aaaa.getline( b'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid aaaa JOIN response')
        return 0

    if ( bbbb.getline( b'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid bbbb JOIN response')
        return 0

    if ( bbbb.getline( b'353 ' + bbbb.nick + b' = ' + cn) == 0 ):
        print('[FAIL] invalid JOIN 353 response')
        return 0

    if ( bbbb.getline( b'366 ' + bbbb.nick + b' ' + cn) == 0 ):
        print('[FAIL] invalid JOIN 366 response')
        return 0

    '''
    if ( bbbb.getline( 'MODE ' + cn + " +tnk") == 0 ):
        print('[FAIL] invalid JOIN MODE response')
        return 0
    '''

    msg = randomstring( random.randint(5,15)).encode('utf-8')

    d = b'PART ' + cn + b' :' + msg + b'\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( b'PART ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid bbbb PART response')
        return 0

    if ( aaaa.getline( b'PART ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PART response')
        return 0

    print('[SUCCESS] set_key')

    return 1

def set_maxusers(aaaa, bbbb ):
    print('[TEST] set_maxusers')

    cn = aaaa.join_new_chan()

    print(b'[INFO] Joined ' + cn)

    ## Set the mode
    d = b'MODE ' + cn + b' +l 1\r\n'

    aaaa.sw(d)

    if ( aaaa.getline( b'MODE ' + cn + b' +l') == 0 ):
        print('[FAIL] invalid MODE +l response')
        return 0

    d = b'JOIN ' + cn + b'\r\n'

    bbbb.sw( d )

    ###:irc.ubuntu-bionic 473 aaaa #bbbb :Cannot join channel (+i) - you must be invited
    if ( bbbb.getline( b'471 ' + bbbb.nick + b' ' + cn + b' :Cannot join' ) == 0):
        print('[FAIL] invalid 473 join fail response')
        return 0

    d = b'MODE ' + cn + b' +l 2\r\n'

    aaaa.sw(d)

    if ( aaaa.getline( b'MODE ' + cn + b' +l') == 0 ):
        print('[FAIL] invalid MODE +l response')
        return 0

    d = b'JOIN ' + cn + b'\r\n'
    bbbb.sw( d )

    if ( aaaa.getline( b'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid aaaa JOIN response')
        return 0

    if ( bbbb.getline( b'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid bbbb JOIN response')
        return 0

    l = bbbb.getaline()

    if l.find(b'332') == -1 and l.find(b'353') == -1:
        print('[FAIL] invalid Join response: %s' %(l))
        return 0

    if ( bbbb.getline( b'366 ' + bbbb.nick + b' ' + cn) == 0 ):
        print('[FAIL] invalid JOIN 366 response')
        return 0
    

    msg = randomstring( random.randint(5,15)).encode('utf-8')

    d = b'PART ' + cn + b' :' + msg + b'\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( b'PART ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid bbbb PART response')
        return 0

    if ( aaaa.getline( b'PART ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PART response')
        return 0

    d = b'MODE ' + cn + b' -l\r\n'

    aaaa.sw(d)

    if ( aaaa.getline( b'MODE ' + cn + b' -l') == 0 ):
        print('[FAIL] invalid MODE -i response')
        return 0

    msg = randomstring( random.randint(5,15)).encode('utf-8')
    d = b'PART ' + cn + b' :' + msg + b'\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( b'PART ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PART response')
        return 0

    aaaa.channels.pop(cn)

    print('[SUCCESS] set_maxusers')

    return 1

def set_moderated(aaaa, bbbb):
    print('[TEST] set_moderated')

    cn = aaaa.join_new_chan()

    print(b'[INFO] Joined ' + cn)

    ## Set the mode
    d = b'MODE ' + cn + b' +m\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( b'MODE ' + cn + b' +m') == 0 ):
        print('[FAIL] invalid MODE +m response')
        return 0

    d = b'JOIN ' + cn + b'\r\n'

    bbbb.sw( d )

    if ( aaaa.getline( b'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid aaaa JOIN response')
        return 0

    if ( bbbb.getline( b'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid bbbb JOIN response')
        return 0

    if ( bbbb.getline( b'353 ' + bbbb.nick + b' = ' + cn + b' :' + bbbb.nick + b' @' + aaaa.nick) == 0 ):
        print('[FAIL] invalid 353 response')
        return 0

    if ( bbbb.getline( b'366 ' + bbbb.nick + b' ' + cn) == 0 ):
        print('[FAIL] invalid JOIN 366 response')
        return 0

    '''
    if ( bbbb.getline( 'MODE ' + cn + " +tnm") == 0 ):
        print('[FAIL] invalid JOIN MODE response')
        return 0

    if ( bbbb.getline( '353 ' + bbbb.nick + " = " + cn) == 0 ):
        print('[FAIL] invalid JOIN 353 response')
        return 0
    '''

    

    msg = randomstring( random.randint(5,15)).encode('utf-8')
    ## Send the message
    ## :irc.ubuntu-bionic 404 bbbb #67 :Cannot send to channel
    d = b'PRIVMSG ' + cn + b' ' + msg + b'\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( b'404 ' + bbbb.nick + b' ' + cn + b' :You') == 0 ):
        print('[FAIL] invalid 404 response')
        return 0
    
    d = b'MODE ' + cn + b' +v ' + bbbb.nick + b'\r\n'
    aaaa.sw(d)

    ##:aaaa!~aaaa@aaa MODE #yolo +v bbbb
    if ( aaaa.getline( b'MODE ' + cn + b' +v ' + bbbb.nick) == 0 ):
        print('[FAIL] invalid aaaa MODE +v response')
        return 0

    if ( bbbb.getline( b'MODE ' + cn + b' +v ' + bbbb.nick) == 0 ):
        print('[FAIL] invalid bbbb MODE +v response')
        return 0

    d = b'PRIVMSG ' + cn + b' ' + msg + b'\r\n'
    bbbb.sw(d)

    if ( aaaa.getline( b'PRIVMSG ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid PRIVMSG response')
        return 0

    d = b'MODE ' + cn + b' -v ' + bbbb.nick + b'\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( b'MODE ' + cn + b' -v ' + bbbb.nick) == 0 ):
        print('[FAIL] invalid aaaa MODE -v response')
        return 0

    if ( bbbb.getline( b'MODE ' + cn + b' -v ' + bbbb.nick) == 0 ):
        print('[FAIL] invalid bbbb MODE -v response')
        return 0

    ## Unset the mode
    d = b'MODE ' + cn + b' -m\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( b'MODE ' + cn + b' -m') == 0 ):
        print('[FAIL] invalid aaaa MODE -m response')
        return 0

    if ( bbbb.getline( b'MODE ' + cn + b' -m') == 0 ):
        print('[FAIL] invalid bbbb MODE -m response')
        return 0

    d = b'PART ' + cn + b' :' + msg + b'\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( b'PART ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid bbbb PART response')
        return 0

    if ( aaaa.getline( b'PART ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PART response')
        return 0

    print('[SUCCESS] set_moderated')

    return 1

def set_private(aaaa, bbbb):
    print('[TEST] set_private')

    cn = random.choice(list(aaaa.channels.keys()))

    ## Set the mode
    d = 'MODE ' + cn + ' +p\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( 'MODE ' + cn + ' +p') == 0 ):
        print('[FAIL] invalid MODE +p response')
        return 0

    d = 'WHOIS ' + aaaa.nick + '\r\n'
    bbbb.sw(d)

    if bbbb.getline('311 ' + bbbb.nick + ' ' + aaaa.nick) == 0:
        print('[FAIL] invalid 311 whois response')
        return 0

    if bbbb.getline('319 ' + bbbb.nick + ' ' + aaaa.nick) == 0:
        print('[FAIL] invalid 319 whois response')
        return 0

    if bbbb.getline('312 ' + bbbb.nick + ' ' + aaaa.nick) == 0:
        print('[FAIL] invalid 312 whois response')
        return 0

    if bbbb.getline('317 ' + bbbb.nick + ' ' + aaaa.nick) == 0:
        print('[FAIL] invalid 317 whois response')
        return 0

    if bbbb.getline('318 ' + bbbb.nick + ' :End of') == 0:
        print('[FAIL] invalid 318 whois response')
        return 0

    d = 'MODE ' + cn + ' -p\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( 'MODE ' + cn + ' -p') == 0 ):
        print('[FAIL] invalid MODE -p response')
        return 0

    print('[SUCCESS] set_private')

    return 1

def set_secret(aaaa, bbbb):
    print('[TEST] set_secret')

    cn = aaaa.join_new_chan()

    ## Set the mode
    d = b'MODE ' + cn + b' +s\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( b'MODE ' + cn + b' +s') == 0 ):
        print('[FAIL] invalid MODE +s response')
        return 0

    d = b'WHO ' + aaaa.nick + b'\r\n'
    bbbb.sw(d)

    if bbbb.getline(b'352 ' + bbbb.nick + b' ') == 0:
        print('[FAIL] invalid 352 who response')
        return 0

    if bbbb.getline(b'315 ' + bbbb.nick + b' ' + aaaa.nick) == 0:
        print('[FAIL] invalid 315 whois response')
        return 0

    d = b'MODE ' + cn + b' -s\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( b'MODE ' + cn + b' -s') == 0 ):
        print('[FAIL] invalid MODE -s response')
        return 0

    msg = randomstring( random.randint(5,15)).encode('utf-8')
    d = b'PART ' + cn + b' :' + msg + b'\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( b'PART ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PART response')
        return 0

    aaaa.channels.pop(cn)

    print('[SUCCESS] set_secret')
    return 1

def set_topic(aaaa, bbbb):
    print('[TEST] set_topic')
    topic = randomstring( random.randint(10,15) ).encode('utf-8')
    msg = randomstring( random.randint(10,15) ).encode('utf-8')

    cn = aaaa.join_new_chan()

    print(b'[INFO] Joined ' + cn)


    d = b'JOIN ' + cn + b'\r\n'
    bbbb.sw( d )

    if ( aaaa.getline( b'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid aaaa JOIN response')
        return 0

    if ( bbbb.getline( b'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid bbbb JOIN response')
        return 0

    if ( bbbb.getline(b'353') == 0 ):
        print('[FAIL] invalid JOIN 353 response')
        return 0

    if ( bbbb.getline(b'366') == 0 ):
        print('[FAIL] invalid JOIN 366 response')
        return 0

    d = b'TOPIC ' + cn + b' ' + topic + b'\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( b'482 ' + bbbb.nick + b' ' + cn + b' :You') == 0 ):
        print('[FAIL] invalid TOPIC 482 response')
        return 0

    d = b'MODE ' + cn + b' +o ' + bbbb.nick + b'\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( b'MODE ' + cn + b' +o') == 0 ):
        print('[FAIL] invalid aaaa MODE +o response')
        return 0

    if ( bbbb.getline( b'MODE ' + cn + b' +o') == 0 ):
        print('[FAIL] invalid bbbb MODE +o response')
        return 0

    d = b'TOPIC ' + cn + b' ' + topic + b'\r\n'
    bbbb.sw(d)

    if ( bbbb.getline(b'TOPIC ' + cn + b' :' + topic) == 0 ):
        print('[FAIL] invalid TOPIC response')
        return 0


    if ( aaaa.getline( b'TOPIC ' + cn + b' :' + topic) == 0 ):
        print('[FAIL] invalid aaaa TOPIC response')
        return 0

    d = b'MODE ' + cn + b' -o ' + bbbb.nick + b'\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( b'MODE ' + cn + b' -o') == 0 ):
        print('[FAIL] invalid aaaa MODE -o response')
        return 0

    if ( bbbb.getline(cn) == 0 ):
        print('[FAIL] invalid bbbb MODE -o response')
        return 0

    d = b'PART ' + cn + b' :' + msg + b'\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( b'PART ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid bbbb PART response')
        return 0

    if ( aaaa.getline( b'PART ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PART response')
        return 0

    msg = randomstring( random.randint(5,15)).encode('utf-8')
    d = b'PART ' + cn + b' :' + msg + b'\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( b'PART ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PART response')
        return 0

    aaaa.channels.pop(cn)

    print('[SUCCESS] set_topic')
    return 1

def send_privmsg( aaaa, bbbb):
    print('[TEST] send_privmsg')

    msg = randomstring( random.randint(10,15)).encode('utf-8')

    d = b'PRIVMSG ' + aaaa.nick + b' :' + msg + b'\r\n'
    bbbb.sw(d)

    if aaaa.getline( b'PRIVMSG ' + aaaa.nick + b' :' + msg ) == 0:
        print('[FAIL] invalid privmsg response')
        return 0

    print('[SUCCESS] send_privmsg')
    return 1

def kickemout( aaaa, bbbb ):
    print('[TEST] kickemout')

    msg = randomstring( random.randint(10,15) ).encode('utf-8')
    cn = aaaa.join_new_chan()

    d = b'JOIN ' + cn + b'\r\n'
    bbbb.sw( d )

    if ( aaaa.getline( b'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid aaaa JOIN response')
        return 0

    if ( bbbb.getline( b'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid bbbb JOIN response')
        return 0

    if ( bbbb.getline( b'353 ' + bbbb.nick + b' = ' + cn) == 0 ):
        print('[FAIL] invalid JOIN 353 response')
        return 0

    if ( bbbb.getline( b'366 ' + bbbb.nick + b' ' + cn) == 0 ):
        print('[FAIL] invalid JOIN 366 response')
        return 0

    '''
        if ( bbbb.getline( 'MODE ' + cn + " +tn") == 0 ):
        print('[FAIL] invalid JOIN MODE response')
        return 0
    '''

    d = b'KICK ' + cn + b' ' + bbbb.nick + b' ' + msg + b'\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( b'KICK ' + cn + b' ' + bbbb.nick + b' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa kick response')
        return 0

    if ( bbbb.getline( b'KICK ' + cn + b' ' + bbbb.nick + b' :' + msg) == 0 ):
        print('[FAIL] invalid bbbb kick response')
        return 0

    msg = randomstring( random.randint(5,15)).encode('utf-8')
    d = b'PART ' + cn + b' :' + msg + b'\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( b'PART ' + cn + b' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PART response')
        return 0

    aaaa.channels.pop(cn)

    print('[SUCCESS] kickemout')
    return 1

def setaway( aaaa, bbbb):
    print('[TEST] setaway')

    msg = randomstring( random.randint(15,20)).encode('utf-8')
    pm = randomstring( random.randint(15,20)).encode('utf-8')

    d = b'AWAY :' + msg + b'\r\n'
    aaaa.sw(d)

    if aaaa.getline(b'306 ' + aaaa.nick + b' :You have been marked as being away') == 0:
        print('[FAIL] Invalid away response')
        return 0

    d = b'PRIVMSG ' + aaaa.nick + b' :' + pm + b'\r\n'
    bbbb.sw(d)

    if bbbb.getline(b'301 ' + bbbb.nick + b' ' + aaaa.nick + b' :' + msg) == 0:
        print('[FAIL] Invalid bbbb away response')
        return 0

    if aaaa.getline(b'PRIVMSG ' + aaaa.nick + b' :' + pm) == 0:
        print('[FAIL] Invalid bbbb privmsg response')
        return 0

    d = b'AWAY\r\n'
    aaaa.sw(d)

    if aaaa.getline(b'305 ' + aaaa.nick + b' :You are no longer marked as being away') == 0:
        print('[FAIL] Invalid away response')
        return 0

    print('[SUCCESS] setaway')
    return 1

def sendison(aaaa, bbbb):
    print('[TEST] sendison')

    d = b'ISON ' + aaaa.nick + b' ' + bbbb.nick + b' ' + randomstring(5).encode('utf-8') + b'\r\n'
    aaaa.sw(d)

    if aaaa.getline(b'303 ' + aaaa.nick + b' :' + aaaa.nick + b' ' + bbbb.nick ) == 0:
        print('[FAIL] Invalid ison response')
        return 0

    print('[SUCCESS] sendison')
    return 1


def sendlist( aaaa, bbbb ):
    print('[TEST] sendlist')

    ## Get them all
    d = 'LIST\r\n'
    aaaa.sw(d)

    print("CAA: ", aaaa.channels)

    if aaaa.getline(b'321 ' + aaaa.nick + b' Channel :Users  Name') == 0:
        print('[FAIL] invalid list response')
        return 0

    t = len(list(aaaa.channels.keys()) + list(bbbb.channels.keys()))

    for _ in range(t):
        if aaaa.getline(b'322 ' + aaaa.nick ) == 0:
            print('[FAIL] invalid list room response')
            return 0

    if aaaa.getline(b':irc.example.org 323 ' + aaaa.nick +  b' :End of /LIST') == 0:
        print('[FAIL] invalid list end response')
        return 0


    print('[SUCCESS] sendlist')
    return 1

def sendnotice( aaaa, bbbb):
    print('[TEST] sendnotice')

    msg = randomstring( random.randint(10,15)).encode('utf-8')

    d = b'NOTICE ' + aaaa.nick + b' ' + msg + b'\r\n'
    bbbb.sw(d)

    if aaaa.getline( b'NOTICE ' + aaaa.nick + b' :' + msg ) == 0:
        print('[FAIL] invalid notice response')
        return 0

    print('[SUCCESS] sendnotice')
    return 1

def sendnames( aaaa, bbbb ):
    print('[TEST] sendnames')

    priva = aaaa.join_new_chan()

    privb = aaaa.join_new_chan()

    d = b'JOIN ' + privb + b'\r\n'
    bbbb.sw( d )

    if ( aaaa.getline( b'JOIN :' + privb) == 0 ):
        print('[FAIL] invalid aaaa JOIN response')
        return 0

    if ( bbbb.getline( b'JOIN :' + privb) == 0 ):
        print('[FAIL] invalid bbbb JOIN response')
        return 0

    if ( bbbb.getline( b'353 ' + bbbb.nick + b' = ' + privb) == 0 ):
        print('[FAIL] invalid JOIN 353 response')
        return 0

    if ( bbbb.getline( b'366 ' + bbbb.nick + b' ' + privb) == 0 ):
        print('[FAIL] invalid JOIN 366 response')
        return 0

    ## bbbb will se all the channels except for the single private one
    t = len( list(aaaa.channels.keys()) + list(bbbb.channels.keys()) )

    d = b'NAMES\r\n'
    bbbb.sw(d)

    if ( bbbb.getline(b':irc.example.org 366 ' + bbbb.nick + b' * :End of /NAMES list' ) == 0):
        print('[FAIL] invalid 366 names response')
        return 0

    msg = randomstring(random.randint( 10,15)).encode('utf-8')

    d = b'PART ' + privb + b' :' + msg + b'\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( b'PART ' + privb + b' :' + msg) == 0 ):
        print('[FAIL] invalid bbbb PART response')
        return 0

    if ( aaaa.getline( b'PART ' + privb + b' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PART response')
        return 0

    print('[SUCCESS] sendnames')
    return 1

def run_test( HOST, PORT ):
    aaaa = ircuser(HOST, PORT)

    if aaaa.loginthing() == 0:
        print('[FAIL] Sending Nick and User did not succeed 0004')
        aaaa.fd.close()
        sys.exit()

    print('[INFO] Login successful. Received server info and MOTD')
    aaaa.do_some_joins()

    print('[INFO] Logging in second user')

    bbbb = ircuser(HOST, PORT)

    if bbbb.loginthing() == 0:
        print('[FAIL] Sending Nick and User did not succeed: 0003')
        bbbb.fd.close()
        sys.exit()

    print('[INFO] Second user logged in')

    allofem = [set_moderated, set_maxusers, sendison, set_key, kickemout, setaway, sendnotice, set_secret, sendlist, sendnames, set_topic, send_privmsg, set_invite]
    now = [set_moderated]

    while len(allofem):
        z = random.choice( allofem )

        allofem.remove(z)

        if z(aaaa, bbbb) == 0:
            print('[FAIL] Test did not succeed')
            aaaa.fd.close()
            bbbb.fd.close()
            sys.exit()

    aaaa.sw("QUIT\r\n")
    bbbb.sw("QUIT\r\n")


    print('[SUCCESS] All tests succeeded')

    aaaa.fd.close()
    bbbb.fd.close()
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
            print('[ERROR] target and port must be specified via arguments or variables.')
            sys.exit(1)

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('[INFO] Poller for the Smith Challenge Binary')
    print('[INFO] SEED: {0}'.format(SEED))

    random.seed( SEED )

    run_test(HOST, PORT)
    print('[COMPLETE] Poller completed successfully')

if __name__ == '__main__':
    main()
