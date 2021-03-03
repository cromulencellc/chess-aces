import sys
import socket
import random
import time
import string
import base64
import os
import datetime

emaillist = [ 
'ibaxgqye@zgjguohr.com',
'udybjtlb@dfyeeulk.com',
'dkyzysnv@fzhzotur.com',
'huwvyetm@zgjguohr.com',
'jygmjtqq@jvfyjuet.com']

loggedin_user = "chess@ubuntu-bionic.com"

## mail from 1
## send from 2
## soml from 3
## saml from 4
send_type = 0

def connect( ip, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (ip, port) )
    except:
        print 'Failed to connect to %s:%d' %(ip, port)
        sys.exit(1)

    return s

def readuntil( s, u ):
    z = ''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1)
    except:
        print 'Failed to receive data'
        return None

    return z

def readline( s ):
    return readuntil( s, '\n')

def send_wrapper(s, data):
    #print data
    s.send(data)

def getbanner( s ):
    try:
        y = readline( s )
    except:
        print '[FAIL] to read banner'
        return 1

    if y.find('CHESS SMPTD') == -1:
        print '[FAIL] Invalid banner received'
        return 1

    return 0

def check_pre_helo_fail( s ):

    try:
        send_wrapper(s,'MAIL FROM:<fail@fail.com>\n')
    except:
        print '[FAIL] Failed to send cphf'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to read cphf response'
        return 1

    if y != '503 A little too soon\n':
        print '[FAIL] Invalid response from cphf'
        return 1
    else:
        print '[SUCCESS] cphf'

    return 0

def check_helo( s ):
    ##check invalid domain
    try:
        send_wrapper(s,'helo invali$d.domain\n')
    except:
        print '[FAIL] Failed to send bad domain ch'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to read bad domain ch'
        return 1

    if y != '501 Domain name fail\n':
        print '[Fail] Invalid bad domain response ch'
        return 1

    try:
        send_wrapper(s,'helo valid.doma.in\n')
    except:
        print '[FAIL] Failed to send valid domain'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] failed to read helo response'
        return 1

    if y.find('Sup valid.doma.in') == -1:
        print '[FAIL] Failed to receive valid helo response'
        return 1
    else:
        print '[SUCCESS] HELO'

    return 0

def check_vrfy( s ):
    ## check invalid address
    try:
        send_wrapper(s,'vrfy invalid@email.com\n')
    except:
        print '[FAIL] Failed to send invalid vrfy email'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to read invalid vrfy email'
        return 1

    if y != '550 Doesn\'t match\n':
        print '[FAIL] Invalid response bad email with domain'
        return 1

    try:
        send_wrapper(s,'vrfy invalidemail\n')
    except:
        print '[FAIL] Failed to send invalid vrfy email'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to read invalid vrfy email'
        return 1

    if y != '550 Doesn\'t match\n':
        print '[FAIL] Invalid response bad email'
        return 1

    dm = random.choice(emaillist)

    try:
        send_wrapper(s,'vrfy %s\n' %(dm) )
    except:
        print '[FAIL] Failed to send valid email vrfy'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to read valid email response'
        return 1

    if y != '250 %s\n' %dm:
        print '[FAIL] Did not receive the expected vrfy response'
        return 1
    else:
        print '[SUCCESS] vrfy %s' %(dm)

    return 0

def check_lexpn( s ):
    try:
        send_wrapper(s, 'lexpn\n')
    except:
        print '[FAIL] Failed to send lexpn command'
        return 1

    y = ''

    while y.find('250 Done') == -1:
        try:
            y = readline(s)
        except:
            print '[FAIL] Failed to read mail list line'
            return 1

    print '[SUCCESS] lexpn'

    return 0

def check_expn( s ):
    try:
        send_wrapper(s,'expn one\n')
    except:
        print '[FAIL] Failed to send expn'
        return 1

    try:
        y = readline( s )
    except:
        print '[FAIL] failed to receive expn response'
        return 1

    if y != '546 Not Authd\n':
        print '[FAIL] Invalid expn response: %s' %(y)
        return 1

    print '[SUCCESS] expn'
    return 0

def check_noop( s ):
    try:
        send_wrapper(s,'noop\n')
    except:
        print '[FAIL] Failed to send noop'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to read noop response'
        return 1

    if y != '250 OK\n':
        print '[FAIL] Invalid noop response'
        return 1

    print '[SUCCESS] noop'

    return 0

def check_mail_from( s ):
    ## assumes that helo has succeeded
    global send_type

    send_type = 1

    ### No ':'
    try:
        send_wrapper(s,'mail from<test@test.com>\n')
    except:
        print '[FAIL] Failed to send mail from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 510 mail from'
        return 1

    if y != '510 Invalid address\n':
        print '[FAIL] Invalid reponse 510 mail from'
        return 1

    ### No '<'
    try:
        send_wrapper(s,'mail from:test@test.com>\n')
    except:
        print '[FAIL] Failed to send mail from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 511 mail from'
        return 1

    if y != '511 Invalid address\n':
        print '[FAIL] Invalid reponse 511 mail from'
        return 1

    ### No '>'
    try:
        send_wrapper(s,'mail from:<test@test.com\n')
    except:
        print '[FAIL] Failed to send mail from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 512 mail from'
        return 1

    if y != '512 Invalid address\n':
        print '[FAIL] Invalid reponse 512 mail from'
        return 1

    ### Bad email format (no @)
    try:
        send_wrapper(s,'mail from:<testtest.com>\n')
    except:
        print '[FAIL] Failed to send mail from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 513 (no @) mail from'
        return 1

    if y != '513 Invalid address\n':
        print '[FAIL] Invalid reponse 513 (no @) mail from'
        return 1

    ### Bad email format (name too long)
    try:
        send_wrapper(s,'mail from:<%s@test.com>\n' %('a'*65))
    except:
        print '[FAIL] Failed to send mail from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 513 (too long name) mail from'
        return 1

    if y != '513 Invalid address\n':
        print '[FAIL] Invalid reponse 513 (too long name) mail from'
        return 1

    ### Bad email format (non alnum)
    try:
        send_wrapper(s,'mail from:<test$@test.com>\n')
    except:
        print '[FAIL] Failed to send mail from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 513 (non alnum) mail from'
        return 1

    if y != '513 Invalid address\n':
        print '[FAIL] Invalid reponse 513 (non alnum) mail from'
        return 1

    ### Bad email format (no .)
    try:
        send_wrapper(s,'mail from:<test@testcom>\n')
    except:
        print '[FAIL] Failed to send mail from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 513 (no dot) mail from'
        return 1

    if y != '513 Invalid address\n':
        print '[FAIL] Invalid reponse 513 (no dot) mail from'
        return 1

    ### Bad email format (long domain)
    try:
        send_wrapper(s,'mail from:<test@%s.com>\n' %('a'*65))
    except:
        print '[FAIL] Failed to send mail from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 513 (long domain) mail from'
        return 1

    if y != '513 Invalid address\n':
        print '[FAIL] Invalid reponse 513 (long domain) mail from'
        return 1

    ### Bad email format (non alum domain)
    try:
        send_wrapper(s,'mail from:<test@te$st.com>\n')
    except:
        print '[FAIL] Failed to send mail from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 513 (non alum domain) mail from'
        return 1

    if y != '513 Invalid address\n':
        print '[FAIL] Invalid reponse 513 (non alum domain) mail from'
        return 1


    ### Bad email format (end too long)
    try:
        send_wrapper(s,'mail from:<test@test.coms>\n')
    except:
        print '[FAIL] Failed to send mail from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 513 (long end) mail from'
        return 1

    if y != '513 Invalid address\n':
        print '[FAIL] Invalid reponse 513 (long end) mail from'
        return 1

    ### Bad email format (non alnum)
    try:
        send_wrapper(s,'mail from:<test@test.c$m>\n')
    except:
        print '[FAIL] Failed to send mail from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 513 (alnum) mail from'
        return 1

    if y != '513 Invalid address\n':
        print y
        print '[FAIL] Invalid reponse 513 (alnum) mail from'
        return 1 

    try:
        send_wrapper(s,'mail from:<test@test.com>\n')
    except:
        print '[FAIL] Failed to send mail from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 250 mail from'
        return 1

    if y != '250 OK\n':
        print '[FAIL] Invalid reponse mail from'
        return 1

    print '[SUCCESS] Mail from'

    return 0

def check_auth_plain_single( s ):
    user = 'gtblfvji'
    p = '29a4b1cbe00cd1a4fc707b79124fdc50'

    ## invalid
    enc = base64.b64encode( 'wrong\x00pass')

    try:
        send_wrapper(s,'auth plain %s\n' %(enc))
    except:
        print '[FAIL] Failed to send auth plain'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Did not receive auth plain response'
        return 1

    if y != '250 Auth Failed\n':
        print '[FAIL] Auth should fail'
        return 1

    ## auth plain on a single line
    enc = base64.b64encode( '%s\x00%s' %(user, p) )

    try:
        send_wrapper(s,'auth plain %s\n' %(enc))
    except:
        print '[FAIL] Failed to send auth plain'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Did not receive auth plain response'
        return 1

    if y != '250 Auth Success\n':
        print '[FAIL] Auth plain single line did not succeed'
        return 1

def check_auth_plain_multi( s ):
    user = 'gtblfvji'
    p = '29a4b1cbe00cd1a4fc707b79124fdc50'

    try:
        send_wrapper(s,'auth plain\n')
    except:
        print '[FAIL] Failed to send auth plain mult'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to read auth plain mult response'
        return 1

    if y != '334\n':
        print '[FAIL] Failed to receive auth plain valid response'
        return 1

    enc = base64.b64encode('%s\x00%s' %(user, p))

    try:
        send_wrapper(s,enc + '\n')
    except:
        print '[FAIL] Failed to send u:p auth multi'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to read u:p response'
        return 1

    if y != '250 Auth Success\n':
        print '[FAIL] Auth did not succeed'
        return 1

    return 0

def check_auth_plain( s ):
    user = 'gtblfvji'
    p = '29a4b1cbe00cd1a4fc707b79124fdc50'

    ## invalid auth
    try:
        send_wrapper(s,'auth letmein\n')
    except:
        print '[FAIL] failed to send auth string'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to read invalid auth response'
        return 1

    if y != '519 Invalid Auth\n':
        print '[FAIL] Did not receive correct invalid auth response'
        return 1


    if check_auth_plain_single(s):
        return 1    

    print '[SUCCESS] Auth Plain (single line)'

    if check_auth_plain_multi(s):
        return 1    

    print '[SUCCESS] Auth Plain (multi line)'

    return 0 

def check_auth_login( s ):
    user = 'gtblfvji'
    p = '29a4b1cbe00cd1a4fc707b79124fdc50'

    try:
        send_wrapper(s,'auth login\n')
    except:
        print '[FAIL] Auth login send'
        return 1

    try:
        y = readline( s )
    except:
        print '[FAIL] Failed to receive b64 user'
        return 1

    if y != '334 VXNlcm5hbWU6\n':
        print '[FAIL] Failed to read user b64'
        return 1

    d = base64.b64encode( user )

    try:
        send_wrapper(s,d + '\n')
    except:
        print '[FAIL] Failed to send b64 user'
        return 1

    try:
        y = readline( s )
    except:
        print '[FAIL] Failed to read pass b64'
        return 1

    if y != '334 UGFzc3dvcmQ6\n':
        print '[FAIL] Failed to receive 64 pass'
        return 1

    d = base64.b64encode( p )

    try:
        send_wrapper(s,d + '\n')
    except:
        print '[FAIL] Failed to send b64 pass'
        return 1

    try:
        y = readline( s )
    except:
        print '[FAIL] Failed to read success'
        return 1

    if y != '250 Auth Success\n':
        print '[FAIL] Failed to read auth login success'
        return 1

    print '[SUCCESS] Auth login'

    return 0

def check_send_from(s):
    global send_type

    send_type = 2

    ### No ':'
    try:
        send_wrapper(s,'send from<test@test.com>\n')
    except:
        print '[FAIL] Failed to send send from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 510 send from'
        return 1

    if y != '510 Invalid address\n':
        print '[FAIL] Invalid reponse 510 send from'
        return 1

    ### No '<'
    try:
        send_wrapper(s,'send from:test@test.com>\n')
    except:
        print '[FAIL] Failed to send send from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 511 send from'
        return 1

    if y != '511 Invalid address\n':
        print '[FAIL] Invalid reponse 511 send from'
        return 1

    ### No '>'
    try:
        send_wrapper(s,'send from:<test@test.com\n')
    except:
        print '[FAIL] Failed to send send from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 512 send from'
        return 1

    if y != '512 Invalid address\n':
        print '[FAIL] Invalid reponse 512 send from'
        return 1

    ### Generic invalid email
    try:
        send_wrapper(s,'send from:<te$st@test.com>\n')
    except:
        print '[FAIL] Failed to send send from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 513 send from'
        return 1

    if y != '513 Invalid address\n':
        print y
        print '[FAIL] Invalid reponse 513 send from'
        return 1

    try:
        send_wrapper(s,'send from:<test@test.com>\n')
    except:
        print '[FAIL] Failed to send send from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 250 send from'
        return 1

    if y != '250 OK\n':
        print '[FAIL] Invalid reponse send from'
        return 1

    print '[SUCCESS] send from'
    
    return 0

def check_soml_from(s):
    global send_type

    send_type = 3

    ### No ':'
    try:
        send_wrapper(s,'soml from<test@test.com>\n')
    except:
        print '[FAIL] Failed to send soml from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 510 soml from'
        return 1

    if y != '510 Invalid address\n':
        print '[FAIL] Invalid reponse 510 soml from'
        return 1

    ### No '<'
    try:
        send_wrapper(s,'soml from:test@test.com>\n')
    except:
        print '[FAIL] Failed to send soml from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 511 soml from'
        return 1

    if y != '511 Invalid address\n':
        print '[FAIL] Invalid reponse 511 soml from'
        return 1

    ### No '>'
    try:
        send_wrapper(s,'soml from:<test@test.com\n')
    except:
        print '[FAIL] Failed to send soml from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 511 soml from'
        return 1

    if y != '511 Invalid address\n':
        print '[FAIL] Invalid reponse 511 soml from'
        return 1

    ### Generic invalid email
    try:
        send_wrapper(s,'soml from:<te$st@test.com\n')
    except:
        print '[FAIL] Failed to send soml from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 511 soml from'
        return 1

    if y != '511 Invalid address\n':
        print '[FAIL] Invalid reponse 511 soml from'
        return 1

    try:
        send_wrapper(s,'soml from:<test@test.com>\n')
    except:
        print '[FAIL] Failed to send soml from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 250 soml from'
        return 1

    if y != '250 OK\n':
        print '[FAIL] Invalid reponse soml from'
        return 1

    print '[SUCCESS] soml from'
    
    return 0

def check_saml_from(s):
    global send_type

    send_type = 4

    ### No ':'
    try:
        send_wrapper(s,'saml from<test@test.com>\n')
    except:
        print '[FAIL] Failed to send saml from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 510 saml from'
        return 1

    if y != '510 Invalid address\n':
        print '[FAIL] Invalid reponse 510 saml from'
        return 1

    ### No '<'
    try:
        send_wrapper(s,'saml from:test@test.com>\n')
    except:
        print '[FAIL] Failed to send saml from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 511 saml from'
        return 1

    if y != '511 Invalid address\n':
        print '[FAIL] Invalid reponse 511 saml from'
        return 1

    ### No '>'
    try:
        send_wrapper(s,'saml from:<test@test.com\n')
    except:
        print '[FAIL] Failed to send saml from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 511 saml from'
        return 1

    if y != '511 Invalid address\n':
        print '[FAIL] Invalid reponse 511 saml from'
        return 1

    ### Generic invalid email
    try:
        send_wrapper(s,'saml from:<te$st@test.com\n')
    except:
        print '[FAIL] Failed to send saml from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 511 saml from'
        return 1

    if y != '511 Invalid address\n':
        print '[FAIL] Invalid reponse 511 saml from'
        return 1

    try:
        send_wrapper(s,'saml from:<test@test.com>\n')
    except:
        print '[FAIL] Failed to send saml from'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to get 250 saml from'
        return 1

    if y != '250 OK\n':
        print '[FAIL] Invalid reponse saml from'
        return 1

    print '[SUCCESS] saml from'
    
    return 0

def check_rcpt_to(s):
    global send_type

    to_list = []



    if send_type in [1, 3 ]:
        cnt = random.randint(1,len(emaillist))

        y = emaillist[:]

        for i in range(cnt):
            d = random.choice(y)

            y.remove(d)

            to_list.append(d)

    if send_type in [2, 3, 4]:
        to_list.append( loggedin_user )

    print to_list

    for x in to_list:
        try:
            send_wrapper(s, 'rcpt to:<%s>\n' %x)
        except:
            print '[FAIL] Failed to add rcpt to %s' %x
            return 1

        try:
            y = readline(s)
        except:
            print '[FAIL] Failed to read rcpt to response'
            return 1

        if y != '250 OK\n':
            print '[FAIL] Invalid rcpt to response: %s' %y
            return 1

    print '[SUCCESS] RCPT TO'

    return 0

def makestr(l):
    z = ''
    while len(z) < l:
        z += random.choice(string.lowercase)

    return z

def check_data(s):
    try:
        send_wrapper(s,'DATA\n')
    except:
        print '[FAIL] Data verb fail'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to read data verb response'
        return 1

    if y != '354 ok\n':
        print '[FAIL] Invalid data verb response: %s' %y
        return 1

    total = 0
    m = random.randint(600, 700)

    print('[INFO] Testing DATA with %d bytes' %m)

    while total < m:
        nl = random.randint(50, 200)

        if total + nl > m:
            nl = m - total
            total = m
        else:
            total += nl

        d = makestr(nl)

        try:
            send_wrapper(s,d + '\n')
        except:
            print '[FAIL] Failed to send data line'
            return 1

    try:
        send_wrapper(s,'.\n')
    except:
        print '[FAIL] Failed to send ending .'
        return 1

    try:
        y = readline(s)
    except:
        print '[FAIL] Failed to read . response'
        return 1

    if y != '250 Message completed.\n':
        print '[FAIL] Invalid . response: %s' %y
        return 1

    print '[SUCCESS] data'
    return 0


## QUIT, rset, help
def main():
    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
    else:
        if 'HOST' in os.environ and 'PORT' in os.environ:
            HOST = os.environ['HOST']
            PORT = int(os.environ['PORT'])
        else:
            print '[ERROR] target and port must be specified via arguments or variables.'
            sys.exit(1)

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print '[INFO] SEED: {0}'.format(SEED)

    random.seed( SEED )

    s = connect( HOST, PORT )

    if getbanner(s):
        print '[FAIL] Poller failed'
        sys.exit(1)
    else:
        print '[SUCCESS] Banner read'

    if check_pre_helo_fail(s):
        sys.exit(1)

    if check_helo(s):
        sys.exit(1)

    if check_vrfy(s):
        sys.exit(1)

    if check_lexpn(s):
        sys.exit(1)

    if check_expn(s):
        sys.exit(1)

    if check_noop(s):
        sys.exit(1)

    mt = random.choice( [check_mail_from, check_send_from, check_saml_from, check_soml_from])
    if mt(s):
        sys.exit(1)

    if check_auth_plain( s ):
        sys.exit(1)

    if check_auth_login ( s ):
        sys.exit(1)

    if check_rcpt_to(s):
        sys.exit(1)

    if check_data(s):
        sys.exit(1)

    s.send('quit\n')
    
    print '[SUCCESS] Poller completed successfully'

    s.close()

if __name__ == '__main__':
    main()
