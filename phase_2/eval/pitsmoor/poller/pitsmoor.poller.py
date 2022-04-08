import sys
import os
import random
import socket
import string
import time

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
    def __init__(self, HOST, PORT):
        self.fd = connect( HOST, PORT)

        self.nick = randomstring(8)
        self.user = randomstring(8)
        self.realname = randomstring(8)

        self.channels = {}

        self.knocksent = 0

    def getaline(self):
        l = readuntil( self.fd, '\r\n').rstrip()

        self.lastline = l

        return l


    def getline( self, expected ):
        l = readuntil( self.fd, '\r\n').rstrip()

        print(l)
        self.lastline = l

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

        z = ['001','002','003','004', '005', '005', '251','254','255','265', '266','250', '422']

        for x in z:
            if self.getline( x ) == 0:
                print('[FAIL] invalid response')
                return 0

        return 1

    def join_new_chan(self):
        cn = randomstring( random.randint(5,10))

        cn = random.choice(['#']) + cn

        d = 'JOIN ' + cn + '\r\n'

        self.sw(d)

        y = readuntil(self.fd, '\r\n').rstrip().split(' ')

        if len(y) < 3:
            print('[FAIL] invalid response length: {}'.format(y))
            return 0

        if y[1] != 'JOIN':
            print('[FAIL] invalid JOIN response: {}'.format(y))
            return 0

        if y[2] != (':' + cn):
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

        if y[4] != (cn):
            print('[FAIL] invalid channel name response: {}'.format(y))
            return 0

        y = readuntil(self.fd, '\r\n').rstrip().split(' ')

        if len(y) < 5:
            print('[FAIL] invalid response length: {}'.format(y))
            return 0

        if y[1] != '366':
            print('[FAIL] invalid 366 response: {}'.format(y))
            return 0            

        self.channels[cn] = '+tn'
        print('[INFO] {} Joined {}'.format(self.nick, cn))

        return cn

    def do_some_joins( self ):
        for i in range( random.randint(2,5)):
            cn = randomstring( random.randint(5,10))

            cn = random.choice(['#', '&']) + cn

            d = 'JOIN ' + cn + '\r\n'

            self.sw(d)

            y = readuntil(self.fd, '\r\n').rstrip().split(' ')

            if len(y) < 3:
                print('[FAIL] invalid response length: {}'.format(y))
                return 0

            if y[1] != 'JOIN':
                print('[FAIL] invalid JOIN response: {}'.format(y))
                return 0

            if y[2] != (':' + cn):
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

            if y[4] != (cn):
                print('[FAIL] invalid channel name response: {}'.format(y))
                return 0

            y = readuntil(self.fd, '\r\n').rstrip().split(' ')

            if len(y) < 5:
                print('[FAIL] invalid response length: {}'.format(y))
                return 0

            if y[1] != '366':
                print('[FAIL] invalid 366 response: {}'.format(y))
                return 0            

            self.channels[cn] = '+tn'
            print('[INFO] {} Joined {}'.format(self.nick, cn))

        print ('[INFO] Channel joins successful')

        return 1

    def sw( self, line):
        self.fd.send(line.encode('utf-8'))

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
    d = 'MODE ' + cn + ' +i\r\n'

    aaaa.sw(d)

    ## :aaaa!~aaa@aaa MODE #weoiru +i
    if ( aaaa.getline( 'MODE ' + cn + ' +i') == 0 ):
        print('[FAIL] invalid MODE +i response')
        return 0

    d = 'JOIN ' + cn + '\r\n'

    bbbb.sw( d )

    ###:irc.ubuntu-bionic 473 aaaa #bbbb :Cannot join channel (+i) - you must be invited
    if ( bbbb.getline( '473 ' + bbbb.nick + ' ' + cn + ' :Cannot join' ) == 0):
        print('[FAIL] invalid 473 join fail response')
        return 0

    ### Send invite
    d = 'INVITE ' + bbbb.nick + ' ' + cn + '\r\n'
    aaaa.sw(d)

    ## :irc.ubuntu-bionic 341 aaaa bbbb #weoiru
    if ( aaaa.getline( '341 ' + aaaa.nick + ' ' + bbbb.nick + ' ' + cn) == 0 ):
        print('[FAIL] invalid invite request response')
        return 0

    if ( bbbb.getline( 'INVITE ' + bbbb.nick + ' ' + cn) == 0 ):
        print('[FAIL] invalid invite send response')
        return 0

    d = 'JOIN ' + cn + '\r\n'

    bbbb.sw( d )

    if ( aaaa.getline( 'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid aaaa JOIN response')
        return 0

    if ( bbbb.getline( 'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid bbbb JOIN response')
        return 0

    if ( bbbb.getline( '353 ' + bbbb.nick + " = " + cn) == 0 ):
        print('[FAIL] invalid JOIN 353 response')
        return 0

    if ( bbbb.getline( '366 ' + bbbb.nick + ' ' + cn) == 0 ):
        print('[FAIL] invalid JOIN 366 response')
        return 0

    

    '''
    if ( bbbb.getline( 'MODE ' + cn + " +itn") == 0 ):
        print('[FAIL] invalid JOIN MODE response')
        return 0
    '''

    

    msg = randomstring( random.randint(5,15))

    d = 'PART ' + cn + ' :' + msg + '\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( 'PART ' + cn + ' :' + msg) == 0 ):
        print('[FAIL] invalid bbbb PART response')
        return 0

    if ( aaaa.getline( 'PART ' + cn + ' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PART response')
        return 0

    d = 'MODE ' + cn + ' -i\r\n'

    aaaa.sw(d)

    if ( aaaa.getline( 'MODE ' + cn + ' -i') == 0 ):
        print('[FAIL] invalid MODE -i response')
        return 0

    print('[SUCCESS] set_invite')

    return 1

def set_key(aaaa, bbbb):
    print('[TEST] set_key')

    msg = randomstring( random.randint(10,20))
    key = randomstring( random.randint(6,10))

    cn = aaaa.join_new_chan()

    print('[INFO] Joined ' + cn)

    ## Set the mode
    d = 'MODE ' + cn + ' +k ' + key + '\r\n'

    aaaa.sw(d)

    ## :aaaa!~aaa@aaa MODE #weoiru +i
    if ( aaaa.getline( 'MODE ' + cn + ' +k ') == 0 ):
        print('[FAIL] invalid MODE +k response')
        return 0

    d = 'JOIN ' + cn + '\r\n'

    bbbb.sw( d )

    if ( bbbb.getline( '475 ' + bbbb.nick + ' ' + cn + ' :Cannot join' ) == 0):
        print('[FAIL] invalid 473 join fail response')
        return 0

    d = 'JOIN ' + cn + ' ' + key + '\r\n'

    bbbb.sw( d )

    if ( aaaa.getline( 'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid aaaa JOIN response')
        return 0

    if ( bbbb.getline( 'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid bbbb JOIN response')
        return 0

    if ( bbbb.getline( '353 ' + bbbb.nick + " = " + cn) == 0 ):
        print('[FAIL] invalid JOIN 353 response')
        return 0

    if ( bbbb.getline( '366 ' + bbbb.nick + ' ' + cn) == 0 ):
        print('[FAIL] invalid JOIN 366 response')
        return 0

    '''
    if ( bbbb.getline( 'MODE ' + cn + " +tnk") == 0 ):
        print('[FAIL] invalid JOIN MODE response')
        return 0
    '''

    msg = randomstring( random.randint(5,15))

    d = 'PART ' + cn + ' :' + msg + '\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( 'PART ' + cn + ' :' + msg) == 0 ):
        print('[FAIL] invalid bbbb PART response')
        return 0

    if ( aaaa.getline( 'PART ' + cn + ' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PART response')
        return 0

    d = 'MODE ' + cn + ' -k\r\n'

    aaaa.sw(d)

    if ( aaaa.getline( 'MODE ' + cn + ' -k *') == 0 ):
        print('[FAIL] invalid MODE -i response')
        return 0

    print('[SUCCESS] set_key')

    return 1

def get_token(aaaa, bbbb):
    print('[TEST] get_token')

    d = 'TOKEN\r\n'

    aaaa.sw(d)

    if (aaaa.getline('denied') == 0 ):
        print('[FAIL] Invalid response')
        return 0

    return 1

def set_maxusers(aaaa, bbbb ):
    print('[TEST] set_maxusers')

    cn = aaaa.join_new_chan()

    print('[INFO] Joined ' + cn)

    ## Set the mode
    d = 'MODE ' + cn + ' +l 1\r\n'

    aaaa.sw(d)

    if ( aaaa.getline( 'MODE ' + cn + ' +l') == 0 ):
        print('[FAIL] invalid MODE +l response')
        return 0

    d = 'JOIN ' + cn + '\r\n'

    bbbb.sw( d )

    ###:irc.ubuntu-bionic 473 aaaa #bbbb :Cannot join channel (+i) - you must be invited
    if ( bbbb.getline( '471 ' + bbbb.nick + ' ' + cn + ' :Cannot join' ) == 0):
        print('[FAIL] invalid 473 join fail response')
        return 0

    d = 'MODE ' + cn + ' +l 2\r\n'

    aaaa.sw(d)

    if ( aaaa.getline( 'MODE ' + cn + ' +l') == 0 ):
        print('[FAIL] invalid MODE +l response')
        return 0

    d = 'JOIN ' + cn + '\r\n'
    bbbb.sw( d )

    if ( aaaa.getline( 'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid aaaa JOIN response')
        return 0

    if ( bbbb.getline( 'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid bbbb JOIN response')
        return 0

    l = bbbb.getaline()

    if l.find('332') == -1 and l.find('353') == -1:
        print('[FAIL] invalid Join response: %s' %(l))
        return 0

    if ( bbbb.getline( '366 ' + bbbb.nick + ' ' + cn) == 0 ):
        print('[FAIL] invalid JOIN 366 response')
        return 0
    

    msg = randomstring( random.randint(5,15))

    d = 'PART ' + cn + ' :' + msg + '\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( 'PART ' + cn + ' :' + msg) == 0 ):
        print('[FAIL] invalid bbbb PART response')
        return 0

    if ( aaaa.getline( 'PART ' + cn + ' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PART response')
        return 0

    d = 'MODE ' + cn + ' -l\r\n'

    aaaa.sw(d)

    if ( aaaa.getline( 'MODE ' + cn + ' -l') == 0 ):
        print('[FAIL] invalid MODE -i response')
        return 0

    print('[SUCCESS] set_maxusers')

    return 1

def set_moderated(aaaa, bbbb):
    print('[TEST] set_moderated')

    cn = aaaa.join_new_chan()

    print('[INFO] Joined ' + cn)

    ## Set the mode
    d = 'MODE ' + cn + ' +m\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( 'MODE ' + cn + ' +m') == 0 ):
        print('[FAIL] invalid MODE +m response')
        return 0

    d = 'JOIN ' + cn + '\r\n'

    bbbb.sw( d )

    if ( aaaa.getline( 'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid aaaa JOIN response')
        return 0

    if ( bbbb.getline( 'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid bbbb JOIN response')
        return 0

    if ( bbbb.getline( '353 ' + bbbb.nick + ' = ' + cn + " :" + bbbb.nick + ' @' + aaaa.nick) == 0 ):
        print('[FAIL] invalid 353 response')
        return 0

    if ( bbbb.getline( '366 ' + bbbb.nick + ' ' + cn) == 0 ):
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

    

    msg = randomstring( random.randint(5,15))
    ## Send the message
    ## :irc.ubuntu-bionic 404 bbbb #67 :Cannot send to channel
    d = 'PRIVMSG ' + cn + ' ' + msg + '\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( '404 ' + bbbb.nick + ' ' + cn + ' :Cannot send') == 0 ):
        print('[FAIL] invalid 404 response')
        return 0
    
    d = 'MODE ' + cn + ' +v ' + bbbb.nick + '\r\n'
    aaaa.sw(d)

    ##:aaaa!~aaaa@aaa MODE #yolo +v bbbb
    if ( aaaa.getline( 'MODE ' + cn + ' +v ' + bbbb.nick) == 0 ):
        print('[FAIL] invalid aaaa MODE +v response')
        return 0

    if ( bbbb.getline( 'MODE ' + cn + ' +v ' + bbbb.nick) == 0 ):
        print('[FAIL] invalid bbbb MODE +v response')
        return 0

    d = 'PRIVMSG ' + cn + ' ' + msg + '\r\n'
    bbbb.sw(d)

    if ( aaaa.getline( 'PRIVMSG ' + cn + ' :' + msg) == 0 ):
        print('[FAIL] invalid PRIVMSG response')
        return 0

    d = 'MODE ' + cn + ' -v ' + bbbb.nick + '\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( 'MODE ' + cn + ' -v ' + bbbb.nick) == 0 ):
        print('[FAIL] invalid aaaa MODE -v response')
        return 0

    if ( bbbb.getline( 'MODE ' + cn + ' -v ' + bbbb.nick) == 0 ):
        print('[FAIL] invalid bbbb MODE -v response')
        return 0

    ## Unset the mode
    d = 'MODE ' + cn + ' -m\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( 'MODE ' + cn + ' -m') == 0 ):
        print('[FAIL] invalid aaaa MODE -m response')
        return 0

    if ( bbbb.getline( 'MODE ' + cn + ' -m') == 0 ):
        print('[FAIL] invalid bbbb MODE -m response')
        return 0

    d = 'PART ' + cn + ' :' + msg + '\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( 'PART ' + cn + ' :' + msg) == 0 ):
        print('[FAIL] invalid bbbb PART response')
        return 0

    if ( aaaa.getline( 'PART ' + cn + ' :' + msg) == 0 ):
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
    d = 'MODE ' + cn + ' +s\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( 'MODE ' + cn + ' +s') == 0 ):
        print('[FAIL] invalid MODE +s response')
        return 0

    d = 'WHO ' + aaaa.nick + '\r\n'
    bbbb.sw(d)

    if bbbb.getline('352 ' + bbbb.nick + ' ') == 0:
        print('[FAIL] invalid 352 who response')
        return 0

    if bbbb.getline('315 ' + bbbb.nick + ' ' + aaaa.nick) == 0:
        print('[FAIL] invalid 315 whois response')
        return 0

    d = 'MODE ' + cn + ' -s\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( 'MODE ' + cn + ' -s') == 0 ):
        print('[FAIL] invalid MODE -s response')
        return 0

    print('[SUCCESS] set_secret')
    return 1

def set_topic(aaaa, bbbb):
    print('[TEST] set_topic')
    topic = randomstring( random.randint(10,15) )
    msg = randomstring( random.randint(10,15) )

    cn = aaaa.join_new_chan()

    print('[INFO] Joined ' + cn)


    d = 'JOIN ' + cn + '\r\n'
    bbbb.sw( d )

    if ( aaaa.getline( 'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid aaaa JOIN response')
        return 0

    if ( bbbb.getline( 'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid bbbb JOIN response')
        return 0

    if ( bbbb.getline('353') == 0 ):
        print('[FAIL] invalid JOIN 353 response')
        return 0

    if ( bbbb.getline('366') == 0 ):
        print('[FAIL] invalid JOIN 366 response')
        return 0

    d = 'TOPIC ' + cn + ' ' + topic + '\r\n'
    bbbb.sw(d)

    if ( bbbb.getline('TOPIC ' + cn + ' :' + topic) == 0 ):
        print('[FAIL] invalid TOPIC response')
        return 0

    '''
    if ( bbbb.getline( '482 ' + bbbb.nick + ' ' + cn + ' :You') == 0 ):
        print('[FAIL] invalid TOPIC 482 response')
        return 0
    '''

    d = 'MODE ' + cn + ' +o ' + bbbb.nick + '\r\n'
    aaaa.sw(d)

    '''
    if ( aaaa.getline( 'MODE ' + cn + ' +o') == 0 ):
        print('[FAIL] invalid aaaa MODE +o response')
        return 0
    '''

    if ( aaaa.getline( 'TOPIC ' + cn + ' :' + topic) == 0 ):
        print('[FAIL] invalid aaaa TOPIC response')
        return 0

    if ( bbbb.getline(cn) == 0 ):
        print('[FAIL] invalid bbbb MODE +o response')
        return 0

    d = 'TOPIC ' + cn + ' ' + topic + '\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( 'TOPIC ' + cn + ' :' + topic) == 0 ):
        print('[FAIL] invalid bbbb TOPIC response')
        return 0

    if ( aaaa.getline( 'MODE ' + cn + ' +o ' + bbbb.nick) == 0 ):
        print('[FAIL] invalid aaaa MODE +o response')
        return 0

    d = 'MODE ' + cn + ' -o ' + bbbb.nick + '\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( 'MODE ' + cn + ' -o') == 0 ):
        print('[FAIL] invalid aaaa MODE -o response')
        return 0

    if ( bbbb.getline(cn) == 0 ):
        print('[FAIL] invalid bbbb MODE -o response')
        return 0

    d = 'PART ' + cn + ' :' + msg + '\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( 'PART ' + cn + ' :' + msg) == 0 ):
        print('[FAIL] invalid bbbb PART response')
        return 0

    if ( aaaa.getline( 'PART ' + cn + ' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PART response')
        return 0

    print('[SUCCESS] set_topic')
    return 1

def set_membersonly(aaaa, bbbb):
    print('[TEST] set_membersonly')
    return 1
    msg = randomstring( random.randint(10,15) )

    cn = random.choice(list(aaaa.channels.keys()))

    d = 'PRIVMSG ' + cn + ' ' + msg + '\r\n'
    print(d)
    bbbb.sw(d)

    if ( aaaa.getline('PRIVMSG ' + cn + ' :' + msg) == 0):
        print("[FAIL] invalid aaa PRIVMSG response")
        return 0

    d = 'MODE ' + cn + ' -n\r\n'
    print(d)
    aaaa.sw(d)

    d = 'PRIVMSG ' + cn + ' ' + msg + '\r\n'
    print(d)
    bbbb.sw(d)

    if ( aaaa.getline( 'PRIVMSG ' + cn + ' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa PRIVMSG response')
        return 0

    d = 'MODE ' + cn + ' +n\r\n'
    print(d)
    aaaa.sw(d)

    if ( aaaa.getline( 'MODE ' + cn + ' +n') == 0 ):
        print('[FAIL] invalid aaaa MODE +n response')
        return 0

    print('[SUCCESS] set_membersonly')
    return 1

def send_privmsg( aaaa, bbbb):
    print('[TEST] send_privmsg')

    msg = randomstring( random.randint(10,15))

    d = 'PRIVMSG ' + aaaa.nick + ' :' + msg + '\r\n'
    bbbb.sw(d)

    if aaaa.getline( 'PRIVMSG ' + aaaa.nick + ' :' + msg ) == 0:
        print('[FAIL] invalid privmsg response')
        return 0

    print('[SUCCESS] send_privmsg')
    return 1

def kickemout( aaaa, bbbb ):
    print('[TEST] kickemout')

    msg = randomstring( random.randint(10,15) )
    cn = aaaa.join_new_chan()

    d = 'JOIN ' + cn + '\r\n'
    bbbb.sw( d )

    if ( aaaa.getline( 'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid aaaa JOIN response')
        return 0

    if ( bbbb.getline( 'JOIN :' + cn) == 0 ):
        print('[FAIL] invalid bbbb JOIN response')
        return 0

    if ( bbbb.getline( '353 ' + bbbb.nick + " = " + cn) == 0 ):
        print('[FAIL] invalid JOIN 353 response')
        return 0

    if ( bbbb.getline( '366 ' + bbbb.nick + ' ' + cn) == 0 ):
        print('[FAIL] invalid JOIN 366 response')
        return 0

    '''
        if ( bbbb.getline( 'MODE ' + cn + " +tn") == 0 ):
        print('[FAIL] invalid JOIN MODE response')
        return 0
    '''

    d = 'KICK ' + cn + ' ' + bbbb.nick + ' ' + msg + '\r\n'
    aaaa.sw(d)

    if ( aaaa.getline( 'KICK ' + cn + ' ' + bbbb.nick + ' :' + msg) == 0 ):
        print('[FAIL] invalid aaaa kick response')
        return 0

    if ( bbbb.getline( 'KICK ' + cn + ' ' + bbbb.nick + ' :' + msg) == 0 ):
        print('[FAIL] invalid bbbb kick response')
        return 0

    print('[SUCCESS] kickemout')
    return 1

def setaway( aaaa, bbbb):
    print('[TEST] setaway')

    msg = randomstring( random.randint(15,20))
    pm = randomstring( random.randint(15,20))

    d = 'AWAY :' + msg + '\r\n'
    aaaa.sw(d)

    if aaaa.getline('306 ' + aaaa.nick + ' :You have been marked as being away') == 0:
        print('[FAIL] Invalid away response')
        return 0

    d = 'PRIVMSG ' + aaaa.nick + ' :' + pm + '\r\n'
    bbbb.sw(d)

    if bbbb.getline('301 ' + bbbb.nick + ' ' + aaaa.nick + ' :' + msg) == 0:
        print('[FAIL] Invalid bbbb away response')
        return 0

    if aaaa.getline('PRIVMSG ' + aaaa.nick + ' :' + pm) == 0:
        print('[FAIL] Invalid bbbb privmsg response')
        return 0

    d = 'AWAY\r\n'
    aaaa.sw(d)

    if aaaa.getline('305 ' + aaaa.nick + ' :You are no longer marked as being away') == 0:
        print('[FAIL] Invalid away response')
        return 0

    print('[SUCCESS] setaway')
    return 1

def sendison(aaaa, bbbb):
    print('[TEST] sendison')

    d = 'ISON ' + aaaa.nick + ' ' + bbbb.nick + ' ' + randomstring(5) + '\r\n'
    aaaa.sw(d)

    if aaaa.getline('303 ' + aaaa.nick + ' :' + aaaa.nick + ' ' + bbbb.nick ) == 0:
        print('[FAIL] Invalid ison response')
        return 0

    print('[SUCCESS] sendison')
    return 1


def sendlist( aaaa, bbbb ):
    print('[TEST] sendlist')

    ## Get them all
    d = 'LIST\r\n'
    aaaa.sw(d)

    if aaaa.getline('321 ' + aaaa.nick + ' Channel :Users  Name') == 0:
        print('[FAIL] invalid list response')
        return 0

    t = len(list(aaaa.channels.keys()) + list(bbbb.channels.keys()))

    for _ in range(t + 1):
        if aaaa.getline('322 ' + aaaa.nick ) == 0:
            print('[FAIL] invalid list room response')
            return 0

    if aaaa.getline('323 ' + aaaa.nick +  ' :End of LIST') == 0:
        print('[FAIL] invalid list end response')
        return 0


    d = 'LIST '

    for x in list(bbbb.channels.keys()):
        d += x + ','

    d = d[:-1] + '\r\n'
    bbbb.sw(d)

    if bbbb.getline('321 ' + bbbb.nick + ' Channel :Users  Name') == 0:
        print('[FAIL] invalid bbbb list response')
        return 0

    for _ in range(t+1):
        if bbbb.getline('322 ' + bbbb.nick ) == 0:
            print('[FAIL] invalid bbbb list room response')
            return 0

    if bbbb.getline('323 ' + bbbb.nick +  ' :End of LIST') == 0:
        print('[FAIL] invalid bbbb list end response')
        return 0

    print('[SUCCESS] sendlist')
    return 1

def sendnotice( aaaa, bbbb):
    print('[TEST] sendnotice')

    msg = randomstring( random.randint(10,15))

    d = 'NOTICE ' + aaaa.nick + ' ' + msg + '\r\n'
    bbbb.sw(d)

    if aaaa.getline( 'NOTICE ' + aaaa.nick + ' :' + msg ) == 0:
        print('[FAIL] invalid notice response')
        return 0

    print('[SUCCESS] sendnotice')
    return 1

def sendnames( aaaa, bbbb ):
    print('[TEST] sendnames')

    priva = aaaa.join_new_chan()

    privb = aaaa.join_new_chan()

    d = 'JOIN ' + privb + '\r\n'
    bbbb.sw( d )

    if ( aaaa.getline( 'JOIN :' + privb) == 0 ):
        print('[FAIL] invalid aaaa JOIN response')
        return 0

    if ( bbbb.getline( 'JOIN :' + privb) == 0 ):
        print('[FAIL] invalid bbbb JOIN response')
        return 0

    '''
    if ( bbbb.getline( 'MODE ' + privb + " +ptn") == 0 ):
        print('[FAIL] invalid JOIN MODE response')
        return 0
    '''

    if ( bbbb.getline( '353 ' + bbbb.nick + " = " + privb) == 0 ):
        print('[FAIL] invalid JOIN 353 response')
        return 0

    if ( bbbb.getline( '366 ' + bbbb.nick + ' ' + privb) == 0 ):
        print('[FAIL] invalid JOIN 366 response')
        return 0

    ## bbbb will se all the channels except for the single private one
    t = len( list(aaaa.channels.keys()) + list(bbbb.channels.keys()) )

    d = 'NAMES\r\n'
    bbbb.sw(d)

    ## 353 aaaa = #dog :@aaaa
    for _ in range(t):
        if ( bbbb.getline( '353 ' + bbbb.nick + ' = ') == 0):
            print('[FAIL] invalid names 353 response')
            return 0

    if ( bbbb.getline('366 ' + bbbb.nick + ' * :End of NAMES list' ) == 0):
        print('[FAIL] invalid 366 names response')
        return 0

    msg = randomstring(random.randint( 10,15))

    d = 'PART ' + privb + ' :' + msg + '\r\n'
    bbbb.sw(d)

    if ( bbbb.getline( 'PART ' + privb + ' :' + msg) == 0 ):
        print('[FAIL] invalid bbbb PART response')
        return 0

    if ( aaaa.getline( 'PART ' + privb + ' :' + msg) == 0 ):
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
    print('[INFO] Logging in second user')

    bbbb = ircuser(HOST, PORT)

    if bbbb.loginthing() == 0:
        print('[FAIL] Sending Nick and User did not succeed: 0003')
        bbbb.fd.close()
        sys.exit()

    print('[INFO] Second user logged in')

    allofem = [sendnotice, sendnames, sendlist, sendison, setaway, kickemout, set_invite, set_key, set_maxusers, get_token, set_moderated, set_secret, set_topic, set_membersonly, send_privmsg]
    now = [sendlist]

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