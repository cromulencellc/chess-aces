import sys
import socket
import random
import time
import base64
import os
import datetime
import struct
import string


pwd = '/home/chess'

### starting environment variables
env = {'USER': 'chess', 'PWD': '/home/chess', 'HOME': '/home/chess'}

### files and their content in /home/chess
files = {'/home/chess/data': 'helloworld\n', '/home/chess/data2': 'chess\nstuff\n'}

### directories below /home/chess
subdirs = []

def connect( ip, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (ip, port) )
    except:
        print 'Failed to connect to %s:%d' %(ip, port)
        sys.exit(1)

    return s

def rs(l):
    z = ''

    for _ in range(l):
        z += random.choice( string.lowercase )

    return z

def readuntil( s, u ):
    z = ''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1)
    except Exception as e:
        print '[ERROR]'
        print (e)
        return None

    return z

def readline( s ):
    return readuntil( s, '\n')

def rd( l ):
    z = ''
    for _ in range(l):
        z += chr( random.randint(1,255))

    return z

def random_string( l ):
    z = ''

    for _ in range(l):
        z += random.choice(string.lowercase)

    return z

def sb( i ):
    return struct.pack('B', i)

def rsb():
    return sb( random.randint(0,255))

def sh( i ):
    return struct.pack('H', i)

def rsh():
    return sh( random.randint(0, 65535))

def si(i):
    return struct.pack('I', i)

def rsi():
    return si( random.randint(0, 2**32 - 1))

def lscmd( s ):
    print '[TEST] running ls command'

    al = random.choice( [0, 1])

    if al:
        s.send( 'ls -alt /home/chess\n')

    else:
        s.send('ls /home/chess\n')


    result = readprompt( s )

    for x in subdirs:
        z = x.rfind('/')

        if z != -1:
            nm = x[z+1:]
        else:
            nm = x

        if result.find(nm) == -1:
            print '[FAILED] Did not find subdir %s' %x
            print result
            return 0

    for x in files:
        z = x.rfind('/')

        if z != -1:
            nm = x[z+1:]
        else:
            nm = x

        if result.find(nm) == -1:
            print '[FAILED] Did not find file: %s' %(x)
            print result
            return 0

    return 1

def trcmd( s ):
    print '[TEST] running tr command'

    nf = random.choice(files.keys())

    nf_data = files[nf]

    #### 0 for delete 1 for translate
    dorr = random.choice([0,1])

    cmd = 'cat ' + nf + ' | tr '

    if dorr:
        l = random.choice(nf_data.rstrip('\n'))

        while l == '"' or l == '\n':
            l = random.choice(nf_data.rstrip('\n'))
        m = random.choice( string.lowercase + string.uppercase)

        trd = nf_data.replace( l, m)

        cmd += '"' + l + '" "' + m + '"'

        s.send( cmd + '\n')

        print '[DEBUG] cmd: %s' %(cmd)

        result = readprompt( s )

        if result.find(trd) == -1:
            print '[ERROR] tr replace failed'
            return 0
    else:
        l = random.choice(nf_data.rstrip('\n'))

        while l == '"' or l == '\n':
            l = random.choice(nf_data.rstrip('\n'))

        trd = nf_data.replace( l, '')

        cmd += '-d "' + l + '"'

        s.send(cmd + '\n')
        print '[DEBUG] cmd: %s' %(cmd)

        result = readprompt( s )

        if result.find(trd) == -1:
            print '[ERROR] tr delete failed'
            return 0

    return 1

def mvcmd( s ):
    print '[TEST] Running mv command'

    y = random.choice(files.keys())

    nn = random_string(7)

    cmd = 'mv ' + y + ' ' + '/home/chess/' + nn

    z = files.pop(y)

    files['/home/chess/' + nn] = z

    s.send(cmd + '\n')

    print '[DEBUG] cmd: %s' %(cmd)

    y = readprompt(s)

    if y.find('mv:') != -1:
        print '[FAILED] mv failed'
        return 0

    return 1

def revcmd( s ):
    print '[TEST] Testing rev command'

    y = random.choice(files.keys())

    data = files[y]

    l = 0
    lines = []
    z = ''

    while l < len(data):
        z += data[l]

        if data[l] == '\n':
            lines.append(z)
            z = ''

        l = l + 1

    if z != '':
        lines.append(z)

    cmd = 'rev %s' %(y)

    print '[DEBUG] cmd: %s' %cmd

    s.send( cmd + '\n')

    for x in lines:
        if x[-1] == '\n':
            nl = 1
            x = x.rstrip('\n')
        else:
            nl = 0

        x = x[::-1]

        for c in x:
            y = s.recv(1)

            if y != c:
                print '[ERROR] Expected %c received %c' %(c, y)
                return 0

        if nl:
            s.recv(1)

    readprompt(s)

    return 1

def cdcmd( s ):
    if len(subdirs) == 0:
        mkdircmd(s)

    print '[TEST] Running cd command'

    y = random.choice( subdirs )

    s.send( 'cd %s\n' %y)
    pwd = y
    env['PWD'] = pwd
    y = readprompt(s)
    if y.find('failed') != -1:
        print '[ERROR] failed to cd'
        return 0


    s.send( 'cd\n')

    pwd = '/home/chess'
    env['PWD'] = pwd
    y = readprompt(s)

    if y.find('failed') != -1:
        print '[ERROR] Failed to cd back'
        return 0

    return 1

def cpcmd( s ):
    if len(subdirs) == 0:
        mkdircmd(s)

    print '[TEST] Running cp command'

    to_rn = random.choice([0,1])

    cpdir = (random.randint(0,100) >90)

    cpmul = (random.randint(0,100) > 90)

    if cpdir:
        print '[DEBUG] copying from directory to directory'

        y = random.choice(subdirs)

        cmd = 'cp -r '

        cmd += y + ' '

        nd = random_string( 8 )

        subdirs.append( nd )

        cmd += nd

        s.send(cmd + '\n')

        print '[DEBUG] cmd: %s ' %(cmd )

        y = readprompt(s)

        if y.find('failed') != -1:
            print '[ERROR] Failed to copy'
            print y
            return 0
    elif cpmul:
        print '[DEBUG] copying multiple files to directory'

        y = random.choice(files.keys())
        z = random.choice(files.keys())

        cmd = 'cp ' + y + ' ' + z + ' '

        w = random.choice(subdirs)
        cmd += w

        print '[DEBUG] cmd: %s ' %(cmd)

        s.send(cmd + '\n')

        y = readprompt(s)

        if y.find('failed') != -1:
            print '[ERROR] Failed to copy'
            print y
            return 0
    else:
        f = random.choice(files.keys())
        d = random.choice(subdirs)

        cmd = 'cp %s %s/' %(f,d)

        if to_rn:
            cmd += random_string(10)

        print '[DEBUG] %s' %(cmd)

        s.send( cmd + '\n' )
        y = readprompt(s)

        if y.find('failed') != -1:
            print '[ERROR] %s' %(y)
            return 0

    return 1

def cmpcmd( s ):
    print '[TEST] Running cmp command'

    a = random.choice(files.keys())
    b = random.choice(files.keys())

    ea = a.rfind('/')
    eb = b.rfind('/')

    if ea != -1:
        nma = a[ea+1:]
    else:
        nma = a

    if eb != -1:
        nmb = b[eb+1:]
    else:
        nmb = b

    a_data = files[a]
    b_data = files[b]

    skp = random.randint(0, 100) > 95

    lmt = random.randint(0, 100) > 95

    pnt = random.randint(0, 100) > 80

    cmd = 'cmp '

    skpa = 0
    skpb = 0
    limit = 0

    if skp:
        bth = random.choice( [0, 1] )

        cmd += '-i '

        if bth:
            skpa = random.randint(1,3)
            skpb = random.randint(1,3)
            cmd += str(skpa) + ':' + str(skpb) + ' '
        else:
            skpa = random.randint(1,3)
            cmd += str(skpa) + ' '

    if lmt:
        limit = random.randint(5, len(a_data))

        cmd += '-n ' + str(random.randint( 5, len(a_data))) + ' '

    if pnt:
        cmd += '-b '

    cmd += a + ' ' + b

    print '[DEBUG] Command: %s' %cmd

    s.send(cmd + '\n')

    sa = a_data[skpa:]
    sb = b_data[skpb:]

    ma = len(sa)
    mb = len(sb)

    if lmt:
        if limit > ma:
            limit = ma

        if limit > mb:
            limit = mb
    else:
        limit = ma

        if limit > mb:
            limit = mb


    for x in range(0, limit):
        if sa[x] != sb[x]:
            final = '%s %s differ after byte %d' %(a, b, x)

            if pnt:
                final += ' is %.2x %c %.2x %c' %(ord(sa[x]), sa[x], ord(sb[x]), sb[x])

            final += '\n'

            result = readline(s)

            if final != result:
                print '[FAILED] Did not receive expected result'
                print 'Expected: %s' %(final)
                print 'Received: %s' %(result)
                return 0

            readprompt(s)

            return 1

    readprompt(s)


def mkdircmd( s ):
    print '[TEST] Running mkdir command'

    verbose = random.randint(0,100) > 80

    setmode = random.randint(0,100) > 95

    cmd = 'mkdir '

    if verbose:
        cmd += '-v '

    if setmode:
        mode = '7' + str(random.randint(0,7)) + str(random.randint(0,7))

        cmd += '-m ' + mode + ' '

    fn = '/home/chess/' + random_string( 8 )

    subdirs.append(fn)

    cmd += fn

    print '[DEBUG] Command: %s' %(cmd)

    s.send(cmd + '\n')

    result = readprompt( s )

    if verbose:
        st = result.find('\nchess@')

        if st == -1:
            print '[ERROR] Some value return error'
            return 0

        expected = "mkdir: created directory '%s'" %(fn)

        if expected != result[:st]:
            print '[ERROR] Failed to read mkdir verbose data'
            return 0

    return 1

def unsetcmd( s ):
    ### can't unset the 3 defaults
    if len(env) == 3:
        return 1

    print '[TEST] Running unset command'

    cmd = 'unset '

    key = random.choice(list(env))

    while key in ['USER', 'PWD', 'HOME']:
        key = random.choice(list(env))

    cmd += key

    s.send(cmd + '\n')

    env.pop(key)

    readprompt(s)

    return 1

def pwdcmd( s ):
    print '[TEST] Running pwd command'

    s.send('pwd\n')

    y = readprompt( s ).split('\n')

    if y[0] != pwd:
        print '[ERROR] Expected pwd is not what was received %s -- %s' %(y[0], pwd)
        return 0

    return 1

def catcmd( s ):
    cmd = 'cat '

    print '[TEST] running cat command'

    fl = random.choice(list(files))

    data = files[fl]

    cmd += fl

    print '[DEBUG] Command: %s' %(cmd)
    s.send( cmd + '\n' )

    result = readprompt(s)

    st = result.find('chess@')

    if st == -1:
        print '[ERROR] Some unknown prompt read error'
        return 0

    dt = result[:st]

    if data != dt:
        print '[ERROR] cat\'ed file data does not match expectations'

        print '%s -- %s' %(data, dt)
        return 0

    return 1

def datecmd( s ):
    print '[TEST] running date command'
    to_file = random.randint( 0, 100) > 90

    s.send('date\n')

    y = readprompt( s )

    return 1

def readprompt( s ):
    p = 'chess@whatever:' + env['PWD'] + '$ '

    return readuntil( s, p )

def checkenv( s ):
    print '[TEST] running env command'
    s.send('env\n')
    y = readprompt(s)

    envresult = y.split('\n')

    if len(envresult) - 1 != len(env):
        print '[FAILED] Environment variable invalid count'
        print envresult
        return 0

    for i in range(len(envresult) - 1):
        st = envresult[i].find('=')

        if st == -1:
            print '[FAILED] Invalid env format'
            return 0

        nm = envresult[i][:st]
        val = envresult[i][st+1:]

        if nm not in env:
            print '[FAILED] %s is not a known variable' %(nm)
            return 0

        if val != env[nm]:
            print '[FAILED] %s is not the correct value for %s' %(val, nm)
            return 0

    return 1

def exportcmd( s ):
    var = random_string( 5 )
    val = random_string( 10 )

    cmd = 'export %s=%s' %(var, val)
    print '[TEST] Running export command: %s' %cmd

    s.send(cmd + '\n')
    readprompt(s)

    env[var] = val

    return 1

def lncmd( s ):
    print '[TEST] running ln command'

    symbolic = random.choice([0,1])

    todir = random.randint(0,100) > 95

    cmd = 'ln '

    if symbolic:
        cmd += '-s '

    nm = random_string(8)

    link = random.choice( files.keys() )

    if todir:
        nd = random_string(8)
        s.send('mkdir /home/chess/' + nd + '\n')
        readprompt(s)

        cmd += '-t ' + nd + ' ' + link

        s.send( cmd + '\n')
        print '[DEBUG] Sending command: %s' %(cmd)

        readprompt(s)
        s.send('ls -al /home/chess/' + nd  + '\n')

        d = readprompt(s)

        if d.find(link[link.rfind('/') + 1:]) == -1:
            print '[FAIL] Failed to create link: %s' %(link[link.rfind('/') + 1:])
            return 0
    else:
        cmd += link + ' ' + nm

        s.send( cmd + '\n')
        print '[DEBUG] Sending command: %s' %(cmd)
        readprompt(s)

        s.send('ls -al /home/chess\n')

        d = readprompt(s)

        if d.find(nm) == -1:
            print '[FAIL] Failed to create link: %s' %nm
            return 0

    return 1

def echo( s ):
    ### just print the help 5%
    print_help = (random.randint(0,100) > 95)

    ## decide if echoing to a file
    to_file = random.choice([0,1])

    if to_file:
        fn = random_string(10) + '.echo'

    ### Decide if we are going to use non ascii
    be = (random.randint(0, 100) > 70)

    ## Trailing newline
    no_trail = random.randint(0, 100) > 70


    cmd = 'echo '

    fin = '\n'

    if no_trail:
        cmd += '-n '
        fin = ''


    if print_help:
        cmd += '-h '
        data = "echo [OPTION]\n\t-n do not output the trailing newline\n\t-e enable interpretation of backslash escapes\n"
    else:
        if be:
            cmd += '-e '
            data = rd(30)

            z = '"'
            for x in data:
                z += '\\x%.2x' %ord(x)
            z += '" '

            cmd += z
            data += fin
        else:
            data = random_string(30)

            cmd += '"' + data + '" '
            data += fin

    if to_file:
        cmd += '>> ' + env['PWD'] + '/' + fn + '\n'

        files[env['PWD'] + '/' + fn] = data

        print '[DEBUG] Command: %s' %(cmd)
        s.send(cmd)
        result = readprompt(s)

        if result.find('failed') != -1:
            print '[ERROR] Failed to write to file \'/home/chess/%s\'' %(fn)
            return 0

    else:
        print '[DEBUG] echo cmd: %s' %(cmd)

        s.send( cmd + '\n')
        result = readprompt(s)

        st = result.find('chess@')

        d = result[:st]

        if data != d:
            print '[ERROR] The data to be echoed was not returned correctly'
            return 0

    return 1

def rmcmd( s ):
    print '[TEST] Running rm command'

    rec = random.randint(0,100) > 80
    verbose = random.choice([0,1])

    cmd = 'rm '

    if verbose:
        cmd += '-v '

    if rec:
        #### Make a bunch of dirs
        top = random_string( 8 )

        s.send( 'mkdir ' + top + '\n')
        readprompt(s)

        sd = []

        for _ in range(5):
            l = random_string(8)

            sd.append(l)

            s.send( 'mkdir ' + top + '/' + l + '\n')
            readprompt(s)

        ### make some files
        for l in sd:
            for _ in range(3):
                nm = random_string(8)

                s.send('echo "aaaaa" >> ' + top + '/' + l + '/' + nm + '\n')
                readprompt(s)

        cmd += '-r ' + top
    else:
        #### Just create a file then remove it
        nm = random_string(8)

        s.send('echo "aaaaa" >> ' + nm + '\n')
        readprompt(s)

        cmd += nm

    print '[DEBUG] Sending command: %s' %(cmd)
    s.send(cmd + '\n')

    y = readprompt(s)

    if y.find('rm:') != -1:
        print '[ERROR] rm failed'
        return 0

    return 1

def run_test(HOST, PORT):
    s = connect( HOST, PORT )

    ### eat the initial prompt
    readprompt(s)

    ## rm, 
    for _ in range(100):
        test_func = random.choice( [rmcmd, lncmd, checkenv, exportcmd, echo, catcmd, datecmd, unsetcmd, pwdcmd, mkdircmd, cdcmd, cpcmd, mvcmd, trcmd, revcmd, lscmd, cmpcmd] )

        r = test_func(s)

        if r == 0:
            print '[FAILED] Test did not succeed'
            s.close()
            sys.exit()

    s.send('exit\n')

    s.close()

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

    print '[INFO] Poller for the Irving Challenge Binary'
    print '[INFO] SEED: {0}'.format(SEED)

    random.seed( SEED )

    run_test(HOST, PORT)
    print '[SUCCESS] Poller completed successfully'

if __name__ == '__main__':
    main()
