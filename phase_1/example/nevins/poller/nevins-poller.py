import sys
import socket
import random
import time
import base64
import os
import datetime
import struct
import string
import traceback
import logging
import subprocess

### these files are expected to be inside of the test_files directory
files = ['chess.jpg', 'faq.html', 'rr.html', 'test.doc', 'test.txt', 'test_del.txt', 'test_del2.txt', 'test_tr.txt']

HOST = ''
PORT = 3004

def connect( ):
    global HOST
    global PORT

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (HOST, PORT) )
    except:
        print('Failed to connect to %s:%d' %(HOST, PORT))
        sys.exit(1)

    time.sleep(.25)
   
    print(s.getsockname())
    s.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 4096*100)
    return s

### wrapper for sending strings
def send_stob( s, data ):
    s.send( bytes( data, 'utf-8' ) )

def readuntil( s, u ):
    z = ''

    try:
        while z.endswith( u ) == False:
            z += str( s.recv(1), 'utf-8')
    except Exception as e:
        print('[ERROR]')
        print (e)
        return None

    return z

def read_file_str( fn ):
    try:
        f = open(fn, 'rb')
    except:
        print('[ERROR] Failed to open ../test_files/' + fn)
        sys.exit(0)

    data = f.read()

    f.close()

    return data

def rs(l):
    z = ''

    for _ in range(l):
        z += random.choice( string.ascii_lowercase )

    return z

def readline( s ):
    return readuntil( s, '\n')

def read_chunk( s, l):
    chunks = []

    bytes_read = 0

    while bytes_read < l:
        try:
            pkt = s.recv(min(l - bytes_read, 4096))
        except Exception as e:
            logging.error(traceback.format_exc())

            print("[ERROR] Connection dropped after reading %d bytes" %(bytes_read))

            #s.close()
            #sys.exit(1)
            bytes_read += len(pkt)

        bytes_read += len(pkt)

        chunks.append(pkt)

    msg = ''.encode('utf-8').join(chunks)

    return msg


def basic_get_request( s ):
    print('[TEST] Basic get request')
    ## pre open the file to compare later
    fn = random.choice( files )

    data = read_file_str('./test_files/' + fn)

    r = 'GET http://127.0.0.1:8000/' + fn + ' HTTP/1.1\r\n'
    r += 'Host: CHESS Poller\r\n'
    r += 'Field: Field Data\r\n'

    r += '\r\n\r\n'

    send_stob(s, r )

    y = readuntil( s, '\r\n\r\n')

    a = y.split('\r\n')

    if a[0].find('OK') == -1:
        print('[ERROR] Failed to get a good response when requesting ' + fn)
        return -1

    length = -1

    for i in range(1, len(a) ):
        d = a[i].split(':')

        if len(d) != 2:
            continue

        if d[0].lower() == 'content-length':
            length = int(d[1])

    if length == -1:
        print('[ERROR] Failed to find the content length')
        return -1

    file_data = read_chunk( s, length )

    if file_data != data:
        print('[ERROR] Failed to receive the expected content data')
        return -1

    print('[INFO] Basic GET request succeeded: ' + fn)

    return 0

def basic_head_request( s ):
    print('[TEST] Basic head request')
    ## pre open the file to compare later
    fn = random.choice( files )

    data = read_file_str('./test_files/' + fn)

    r = 'HEAD http://127.0.0.1:8000/' + fn + ' HTTP/1.1\r\n'
    r += 'Host: CHESS Poller\r\n'
    r += 'Field: Field Data\r\n'

    r += '\r\n\r\n'

    send_stob(s, r )

    y = readuntil( s, '\r\n\r\n')

    a = y.split('\r\n')

    if a[0].find('OK') == -1:
        print('[ERROR] Failed to get a good response when requesting ' + fn)
        return -1

    length = -1

    for i in range(1, len(a) ):
        d = a[i].split(':')

        if len(d) != 2:
            continue

        if d[0].lower() == 'content-length':
            length = int(d[1])

    if length == -1:
        print('[ERROR] Failed to find the content length')
        return -1

    if length != len(data):
        print('[ERROR] Invalid content length')
        return -1

    print('[INFO] Basic HEAD request succeeded: ' + fn)

    return 0

def get_request_with_cut( s ):
    print ('[TEST] Get request with cut')
    
    ## first we need to create the filter
    ft = 'GET /filter HTTP/1.1\r\n'
    ft += 'Filter: B cut script\r\n\r\n'

    send_stob(s, ft )

    s.close()

    try:
        s = connect()
    except Exception as e:
        logging.error(traceback.format_exc())

    ## Select an html file
    fn = ''

    while fn.endswith('html') != True:
        fn = random.choice( files )

    fn_path = './test_files/' + fn

    print ("[INFO] Requesting: %s" %(fn))

    ## Run the file through the http utility to get the response
    ## I always just strip the script element

    result = ''
    try:
        result = subprocess.check_output( ['./http', fn_path, "script"] )
    except Exception as e:
        logging.error(traceback.format_exc())

        print (result)
        sys.exit(1)

    data = read_file_str('./out.html')

    print("LENGTH of data: %d" %(len(data)))

    r = 'GET http://127.0.0.1:8000/' + fn + ' HTTP/1.1\r\n'
    r += 'Host: CHESS Poller\r\n'
    r += 'Field: Field Data\r\n'

    r += '\r\n\r\n'

    send_stob(s, r )

    y = readuntil( s, '\r\n\r\n')

    a = y.split('\r\n')

    if a[0].find('OK') == -1:
        print ('[ERROR] Failed to get a good response when requesting ' + fn)
        return -1

    length = -1

    for i in range(1, len(a) ):
        d = a[i].split(':')

        if len(d) != 2:
            continue

        if d[0].lower() == 'content-length':
            length = int(d[1])

    if length == -1:
        print('[ERROR] Failed to find the content length')
        return -1

    file_data = read_chunk( s, length )

    if file_data != data:
        print ('[ERROR] Received length is: %d' %len(file_data))
        print ('[ERROR] Expected length is: %d' %len(data))
        print ('[ERROR] Failed to receive the expected content data')
        for x in range(len(file_data)):
            if file_data[x] != data[x]:
                print(file_data[x:x+20])
                print("-"*20)
                print(data[x:x+20])
            return -1
        return -1

    print ('[INFO] Get request with cut succeeded: ' + fn)

    return 0

def request_with_mime( s ):
    print ('[TEST] Request with mime')
    ## pre open the file to compare later
    fn = random.choice( files )

    data = read_file_str('./test_files/' + fn)

    r = 'GET http://127.0.0.1:8000/' + fn + ' HTTP/1.1\r\n'
    r += 'Host: CHESS Poller\r\n'
    r += 'Field: Field Data\r\n'

    r += '\r\n\r\n'

    send_stob(s, r )

    y = readuntil( s, '\r\n\r\n')

    a = y.split('\r\n')

    if a[0].find('OK') == -1:
        print ('[ERROR] Failed to get a good response when requesting ' + fn)
        return -1

    length = -1

    for i in range(1, len(a) ):
        d = a[i].split(':')

        if len(d) != 2:
            continue

        if d[0].lower() == 'content-length':
            length = int(d[1])

    if length == -1:
        print ('[ERROR] Failed to find the content length')
        return -1

    file_data = read_chunk(s, length)

    if file_data != data:
        print ('[ERROR] Failed to receive the expected content data: mime')
        return -1

    mt = None

    for i in range(1, len(a) ):
        d = a[i].split(':')

        if len(d) != 2:
            continue

        if d[0].lower() == 'content-type':
            mt = d[1]

    if mt == None:
        print ('[ERROR] Failed to locate content type')
        return -1

    sp = mt.lstrip().split('/')

    print ('Type: ' + sp[0] + ' Subtype: ' + sp[1])

    if fn.find('.txt') != -1:
        if sp[0] != 'text' or sp[1] != 'plain':
            print ('[ERROR] Expected text/plain for file ' + fn)
            return -1
    elif fn.find('.doc') != -1:
        if sp[0] != 'application' or sp[1] != 'msword':
            print ('[ERROR] Expected application/msword for file ' + fn)
            return -1
    elif fn.find('.jpeg') != -1:
        if sp[0] != 'image' or sp[1] != 'jpeg':
            print ('[ERROR] Expected image/jpeg for file ' + fn)
            return -1
    elif fn.find('.html') != -1:
        if sp[0] != 'text' or sp[1] != 'html':
            print ('[ERROR] Expected image/jpeg for file ' + fn)
            return -1

    print ('[INFO] Mime check succeeded: ' + fn)

    return 0

def delete_filter( s ):
    ft = 'GET /filter HTTP/1.1\r\n'
    ft += 'Filter: B drp 0\r\n\r\n'

    send_stob(s,  ft )

    return 0

def tr_filt( s ):

    ### Lets just translate a random word from rr.html
    data = read_file_str('./test_files/rr.html')

    z = data.split(' '.encode('utf-8'))

    word = ''

    while len(word) < 5 or len(word) > 10:
        word = random.choice(z)

    rep = bytes(rs( random.randint(5,10)), 'utf-8')

    print ('[TEST] Testing tr filter: %s -> %s' %(word, rep))

    ft = 'GET /filter HTTP/1.1\r\n'
    ft += 'Filter: B tr %s %s\r\n\r\n' %(word.decode('ascii'), rep.decode('ascii'))

    send_stob(s, ft )

    s.close()

    s = connect()

    data = data.replace( word, rep )

    req = 'GET 127.0.0.1:8000/rr.html HTTP/1.1\r\n'
    req += 'Host: CHESS Poller\r\n\r\n'

    send_stob(s,req)

    y = readuntil( s, '\r\n\r\n')

    a = y.split('\r\n')

    if a[0].find('OK') == -1:
        print ('[ERROR] Failed to get a good response when requesting rr.html')
        print (a)
        return -1

    length = -1

    for i in range(1, len(a) ):
        d = a[i].split(':')

        if len(d) != 2:
            continue

        if d[0].lower() == 'content-length':
            length = int(d[1])

    if length == -1:
        print ('[ERROR] Failed to find the content length')
        return -1

    file_data = read_chunk(s, length)

    if file_data != data:

        print ('fdl: %d d: %d' %(len(file_data), len(data)))

        for i in range(len(file_data)):
            if file_data[i] != data[i]:
                print (file_data[i:i+50])
                print ('-'*20)
                print (data[i:i+50])
                break

        print ('[ERROR] Failed to receive the expected content data')
        return -1

    return 0

def del_filt(s):
    ### Lets just translate a random word from rr.html

    data = read_file_str('./test_files/rr.html')

    z = data.split(' '.encode('utf-8'))

    word = ''

    while len(word) < 5 or len(word) > 10:
        word = random.choice(z)

    print ('[TEST] Testing del filter: %s' %(word))

    ft = 'GET /filter HTTP/1.1\r\n'
    ft += 'Filter: B del {0}\r\n\r\n'.format( word.decode('ascii'))

    send_stob( s, ft )

    s.close()

    s = connect()

    data = data.replace( word, ''.encode('utf-8'))

    req = 'GET 127.0.0.1:8000/rr.html HTTP/1.1\r\n'
    req += 'Host: CHESS Poller\r\n\r\n'

    send_stob(s, req)

    y = readuntil( s, '\r\n\r\n')

    a = y.split('\r\n')

    if a[0].find('OK') == -1:
        print ('[ERROR] Failed to get a good response when requesting')
        return -1

    length = -1

    for i in range(1, len(a) ):
        d = a[i].split(':')

        if len(d) != 2:
            continue

        if d[0].lower() == 'content-length':
            length = int(d[1])

    if length == -1:
        print ('[ERROR] Failed to find the content length')
        return -1

    file_data = read_chunk(s, length);

    if file_data != data:
        print ('[ERROR] Failed to receive the expected content data')

        print (file_data[:30])
        print (data[:30])
        for x in range(20):
            print("%x -- %x" %(ord(file_data[x]), ord(data[x])))
        return -1

    return 0
    
def blk_filt(s):
    ### Lets just block on a random word from rr.html
    data = read_file_str('./test_files/rr.html')

    z = data.split(' '.encode('utf-8'))

    word = ''

    while len(word) < 5 or len(word) > 10:
        word = random.choice(z)
        try:
            d = word.decode('ascii')
        except:
            print('Exception with word')
            word = ''
            continue

    print ('[TEST] Testing blk filter: %s' %(word).decode('ascii'))

    ft = 'GET /filter HTTP/1.1\r\n'
    ft += 'Filter: B blk %s\r\n\r\n' %(word.decode('ascii'))

    send_stob(s,  ft )

    s.close()

    s = connect()

    data = data.replace( word, ''.encode('utf-8'))

    req = 'GET 127.0.0.1:8000/rr.html HTTP/1.1\r\n'
    req += 'Host: CHESS Poller\r\n\r\n'

    send_stob(s, req)

    y = readuntil( s, '\r\n\r\n')

    a = y.split('\r\n')

    if a[0].find('filtered') == -1:
        print ('[FAILED] The blk filter was not successful')
        return -1
    else:
        print ('[SUCCESS] The request was successfully filtered')

    return 0

def hh_filt(s):
    ### block on Content-Type header
    print ('[TEST] Testing hh filter')

    ft = 'GET /filter HTTP/1.1\r\n'
    ft += 'Filter: B hh content-type\r\n\r\n'

    send_stob(s, ft )

    s.close()

    s = connect()

    req = 'GET 127.0.0.1:8000/rr.html HTTP/1.1\r\n'
    req += 'Host: CHESS Poller\r\n\r\n'

    send_stob(s, req)

    y = readuntil( s, '\r\n\r\n')

    a = y.split('\r\n')

    if a[0].find('filtered') == -1:
        print ('[FAILED] The hh filter was not successful')
        return -1
    else:
        print ('[SUCCESS] The request was successfully filtered')

    return 0

def tgt_filt(s):
    ### block on Content-Type header
    print ('[TEST] Testing tgt filter: localhost')

    ft = 'GET /filter HTTP/1.1\r\n'
    ft += 'Filter: B tgt localhost\r\n\r\n'

    send_stob(s, ft )

    s.close()

    s = connect()

    req = 'GET localhost:8000/rr.html HTTP/1.1\r\n'
    req += 'Host: CHESS Poller\r\n\r\n'

    send_stob(s,req)

    y = readuntil( s, '\r\n\r\n')

    a = y.split('\r\n')

    if a[0].find('blocked') == -1:
        print ('[FAILED] The tgt filter was not successful')
        return -1
    else:
        print ('[SUCCESS] The request was successfully filtered')

    return 0

def src_filt(s):
    ### block on Content-Type header
    print ('[TEST] Testing src filter: localhost')

    ft = 'GET /filter HTTP/1.1\r\n'
    ft += 'Filter: B src CHESS Poller\r\n\r\n'

    send_stob(s, ft )

    s.close()

    s = connect()

    req = 'GET localhost:8000/rr.html HTTP/1.1\r\n'
    req += 'Host: CHESS Poller\r\n\r\n'

    send_stob(s,req)

    y = readuntil( s, '\r\n\r\n')

    a = y.split('\r\n')

    if a[0].find('blocked') == -1:
        print ('[FAILED] The tgt filter was not successful')
        return -1
    else:
        print ('[SUCCESS] The request was successfully filtered')

    return 0

def bft_filt(s):
    print ('[TEST] Testing bft filter: html')

    ft = 'GET /filter HTTP/1.1\r\n'
    ft += 'Filter: P bft html\r\n\r\n'

    send_stob(s, ft )

    s.close()

    s = connect()

    req = 'GET 127.0.0.1:8000/rr.html HTTP/1.1\r\n'
    req += 'Host: CHESS Poller\r\n\r\n'

    send_stob(s,req)

    y = readuntil( s, '\r\n\r\n')

    a = y.split('\r\n')

    if a[0].find('blocked') == -1:
        print ('[FAILED] The blk filter was not successful')
        return -1
    else:
        print ('[SUCCESS] The request was successfully filtered')

    return 0

def mmt_filt( s ):
    print ('[TEST] Testing mm_t filter: text')

    ft = 'GET /filter HTTP/1.1\r\n'
    ft += 'Filter: S mm_t text\r\n\r\n'

    send_stob(s, ft )

    s.close()

    s = connect()

    req = 'GET 127.0.0.1:8000/rr.html HTTP/1.1\r\n'
    req += 'Host: CHESS Poller\r\n\r\n'

    send_stob(s,req)

    y = readuntil( s, '\r\n\r\n')

    a = y.split('\r\n')

    if a[0].find('filtered') == -1:
        print ('[FAILED] The mm_t filter was not successful')
        return -1
    else:
        print ('[SUCCESS] The request was successfully filtered')

    return 0

def mms_filt( s ):
    print ('[TEST] Testing mm_s filter: html')

    ft = 'GET /filter HTTP/1.1\r\n'
    ft += 'Filter: S mm_s html\r\n\r\n'

    send_stob(s,  ft )

    s.close()

    s = connect()

    req = 'GET 127.0.0.1:8000/rr.html HTTP/1.1\r\n'
    req += 'Host: CHESS Poller\r\n\r\n'

    send_stob(s, req)

    y = readuntil( s, '\r\n\r\n')

    a = y.split('\r\n')

    if a[0].find('filtered') == -1:
        print ('[FAILED] The mm_s filter was not successful')
        return -1
    else:
        print ('[SUCCESS] The request was successfully filtered')

    return 0

def run_test( ):
    ### do some regular requests
    global HOST
    global PORT

    for _ in range( random.randint(10,30) ):
        c = random.choice( [get_request_with_cut, basic_get_request, basic_head_request, request_with_mime, tr_filt, del_filt, blk_filt, hh_filt, tgt_filt, src_filt, bft_filt, mmt_filt, mms_filt])
        
        s = connect( )

        r = c(s)

        s.close()

        if r == -1:
            print ('[FAILED] Quitting')
            return -1

        s = connect()
        delete_filter(s)
        s.close()

        time.sleep(.25)

    return 0

def main():
    global HOST
    global PORT

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

    print('[INFO] Poller for the Nevins Challenge Binary')
    print('[INFO] This poller makes the assumption that the HTTP server is on the same device as the challenge on port 8000')
    print('[INFO] SEED: {0}'.format(SEED))

    random.seed( SEED )

    if os.path.isfile('http') != True:
        print('[ERROR] The poller needs the http utility. Please run \'make\'')
        sys.exit(1)

    if run_test( ) != -1:
        print('[SUCCESS] Poller completed successfully')
        sys.exit(0)

    sys.exit(1)


if __name__ == '__main__':
    main()
