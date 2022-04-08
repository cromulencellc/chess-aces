import sys
import socket
import random
import time
import string
import base64
import os
import datetime
import string
import signal
import subprocess
import copy

def handler( signum, frame):
    print('\t[FAIL] sigalrm fired Poller failed')
    exit(1)

def randomstring(l):
    q = ''

    for _ in range(l):
        q += random.choice(string.ascii_lowercase)

    return q

def connect( ip, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (ip, port) )
    except:
        print('Failed to connect to %s:%d' %(ip, port))
        sys.exit(1)

    return s

def RU( s, u ):
    z = ''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1).decode('utf-8')
    except Exception as e:
        print('Failed to receive data')
        print(e)
        return None

    return z

def RL( s ):
    return RU( s, '\n')

def SW(s, data):
    f = open('log.txt', 'a')
    f.write(data.decode('utf-8'))
    f.close()
    s.sendall(data)

def SC( s, data ):

    while len(data):
        if len(data) > 1000:
            s.send(data[:1000])
            data = data[1000:]
            RL(s)
        else:
            s.send(data)
            data = ''

def RB(s):
    return RU(s, '# ')

def long_verb(s):
    print('[TEST] long_verb')

    verb = randomstring( random.randint( 16,20) )

    req = verb.encode('UTF-8') + b' /index.html HTTP/1.1\r\n'
    req += b'HOST: chess.com\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    if y.find(b'Message: Bad Request') == -1:
        return 1

    print('\t[SUCCESS] long_verb')

    return 0

def bad_verb( s ):
    print('[TEST] bad_verb')

    verb = randomstring( random.randint( 3,8) )

    req = verb.encode('UTF-8') + b' /index.html HTTP/1.1\r\n'
    req += b'HOST: chess.com\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    if y.find(b'Message: Not Implemented') == -1:
        return 1

    print('\t[SUCCESS] bad_verb')

    return 0

def no_space( s ):
    print('[TEST] no_space')

    verb = randomstring( random.randint( 3,8) )

    req =  b'GET /index.htmlHTTP/1.1\r\n'
    req += b'HOST: chess.com\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Bad Request') == -1:
        return 1

    print('\t[SUCCESS] no_space')

    return 0

def norn( s ):
    print('[TEST] norn')

    req =  b'GET /index.html HTTP/1.1\n\n'
    req += b'HOST: chess.com\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Bad Request') == -1:
        return 1

    print('\t[SUCCESS] norn')

    return 0

def bad_prot( s ):
    print('[TEST] bad_prot')

    req =  b'GET /index.html ' + randomstring( 4 ).encode('utf-8') + b'/1.1\r\n'
    req += b'HOST: chess.com\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Bad Request') == -1:
        return 1

    print('\t[SUCCESS] bad_prot')

    return 0

def bad_major( s ):
    print('[TEST] bad_major')

    req =  b'GET /index.html HTTP/' + str(random.randint(3,9)).encode('utf-8') + b'.1\r\n'
    req += b'HOST: chess.com\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: HTTP Version Not Supported') == -1:
        return 1

    print('\t[SUCCESS] bad_major')

    return 0

def bad_minor( s ):
    print('[TEST] bad_minor')

    req =  b'GET /index.html HTTP/' + str(random.randint(1,2)).encode('utf-8') + b'.' + str(random.randint(2,9)).encode('utf-8') +b'\r\n'
    req += b'HOST: chess.com\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: HTTP Version Not Supported') == -1:
        return 1

    print('\t[SUCCESS] bad_minor')

    return 0

def nocolon( s ):
    print('[TEST] nocolon')

    req =  b'GET /index.html HTTP/1.1\r\n'
    req += randomstring(5).encode('utf-8') + b' ' + randomstring(10).encode('utf-8') + b'\r\n'
    req += b'HOST: chess.com\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Bad Request') == -1:
        return 1

    print('\t[SUCCESS] nocolon')

    return 0

def no_field_name( s ):
    print('[TEST] no_field_name')

    req =  b'GET /index.html HTTP/1.1\r\n'
    req += b': ' + randomstring(10).encode('utf-8') + b'\r\n'
    req += b'HOST: chess.com\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Bad Request') == -1:
        return 1

    print('\t[SUCCESS] no_field_name')

    return 0

def no_field_data( s ):
    print('[TEST] no_field_data')

    req =  b'GET /index.html HTTP/1.1\r\n'
    req += randomstring(5).encode('utf-8') + b': \r\n'
    req += b'HOST: chess.com\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Bad Request') == -1:
        return 1

    print('\t[SUCCESS] no_field_data')

    return 0

def encodings( s ):
    print('[TEST] encodings')

    req =  b'GET /index.html HTTP/1.1\r\n'    

    valid_encodings = [b'gzip', b'compress', b'deflate', b'bzip2' ]

    enc_count = random.randint(1, 3)

    for x in range(enc_count):
        if random.randint(0,100) > 80:
            enc = randomstring( 5 ).encode('utf-8')
        else:
            enc = random.choice(valid_encodings)

        req += b'Accept-Encoding: '

        req += enc

        if random.randint(0, 100) > 70:
            req += b';q='

            v = round(random.uniform(0, 1), 2)

            req += str(v).encode('utf-8')

        req += b'\r\n'

    req += b'Host: chess.com\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    ## TODO confirm y is index.html

    print('\t[SUCCESS] encodings')

    return 0

def inject_dotdot_slash( uri ):

    slash_count = uri.count(b'/')
    choice = random.randint(1, slash_count)

    beg = 0

    for x in range(choice):
       beg = uri.find(b"/", beg) + 1

    folder = randomstring(5).encode('utf-8')

    uri = uri[:beg] + folder + b'/../' + uri[beg:]

    return uri

def inject_dot_slash( uri ):

    slash_count = uri.count(b'/')
    choice = random.randint(1, slash_count)

    beg = 0

    for x in range(choice):
       beg = uri.find(b"/", beg) + 1

    uri = uri[:beg] + b'./' + uri[beg:]

    return uri

def inject_pcnts( uri ):
    final = b''

    for x in uri:
        if random.randint(0,100) > 90:
            final += b'%' + hex(x)[2:].encode('utf-8')
        else:
            final += chr(x).encode('utf-8')

    return final

def urlencoding( s ):
    print('[TEST] urlencoding')

    ## inject some './' and some ../

    uri = b'/index.html'

    funcs = [inject_dot_slash, inject_dotdot_slash]

    for i in range(5):
        f = random.choice(funcs)
        uri = f(uri)
        print(uri)

    uri = inject_pcnts(uri)

    req =  b'GET ' + uri + b' HTTP/1.1\r\n'
    req += b'HOST: chess.com\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    ## TODO confirm it is good
    print(y)

    print('\t[SUCCESS] urlencoding')

    return 0

def parse_errors_test( s ):
    print('[TEST] parse_errors_test')

    funcs = [urlencoding, encodings, no_field_data, no_field_name, nocolon, bad_minor, bad_major, bad_prot, norn, long_verb, bad_verb, no_space]

    f = random.choice(funcs)

    return f(s)

def nohost( s ):
    print('[TEST] nohost')

    req =  b'GET /index.html HTTP/1.1\r\n'
    req += randomstring(5).encode('utf-8') + b': ' + randomstring(10).encode('utf-8') + b'\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Bad Request') == -1:
        return 1

    print('\t[SUCCESS] nohost')

    return 0

def multihost( s ):
    print('[TEST] multihost')

    req =  b'GET /index.html HTTP/1.1\r\n'
    req += b'Host: chess\r\n'
    req += b'Host: ' + randomstring(5).encode('utf-8') + b'\r\n'
    req += randomstring(5).encode('utf-8') + b': ' + randomstring(10).encode('utf-8') + b'\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Bad Request') == -1:
        return 1

    print('\t[SUCCESS] multihost')

    return 0

def no_start_slash( s ):
    print('[TEST] no_start_slash')

    req =  b'GET index.html HTTP/1.1\r\n'
    req += b'Host: chess\r\n'
    req += randomstring(5).encode('utf-8') + b': ' + randomstring(10).encode('utf-8') + b'\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Bad Request') == -1:
        return 1

    print('\t[SUCCESS] no_start_slash')

    return 0

def file_not_exist(s):
    print('[TEST] file_not_exist')

    uri = b'/' + randomstring(5).encode('utf-8') + b'.html'

    req =  b'GET ' + uri +b' HTTP/1.1\r\n'
    req += b'Host: chess\r\n'
    req += randomstring(5).encode('utf-8') + b': ' + randomstring(10).encode('utf-8') + b'\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Not Found') == -1:
        return 1

    print('\t[SUCCESS] file_not_exist')

    return 0

def forbidden(s):
    print('[TEST] forbidden')

    uri = b'/'

    req =  b'GET ' + uri +b' HTTP/1.1\r\n'
    req += b'Host: chess\r\n'
    req += randomstring(5).encode('utf-8') + b': ' + randomstring(10).encode('utf-8') + b'\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Forbidden') == -1:
        return 1

    print('\t[SUCCESS] forbidden')

    return 0

def invalid_ifmodsince( s ):
    print('[TEST] invalid_ifmodsince')

    uri = b'/index.html'

    req =  b'GET ' + uri +b' HTTP/1.1\r\n'
    req += b'Host: chess\r\n'
    req += b'If-Modified-Since: ' + time.strftime( "%a, %d %b %Y %H:%M:%S", time.gmtime()).encode('utf-8') + b'\r\n'
    req += randomstring(5).encode('utf-8') + b': ' + randomstring(10).encode('utf-8') + b'\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    content = s.recv(length)

    print(y)

    if y.find(b'200 OK') == -1:
        return 1

    print('\t[SUCCESS] invalid_ifmodsince')

    return 0

def notmodified(s):
    print('[TEST] notmodified')

    uri = b'/index.html'

    req =  b'GET ' + uri +b' HTTP/1.1\r\n'
    req += b'Host: chess\r\n'
    req += b'If-Modified-Since: ' + time.strftime( "%a, %d %b %Y %H:%M:%S GMT", time.gmtime()).encode('utf-8') + b'\r\n'
    req += randomstring(5).encode('utf-8') + b': ' + randomstring(10).encode('utf-8') + b'\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    if y.find(b'304 Not Modified') == -1:
        return 1

    print('\t[SUCCESS] notmodified')

    return 0

def get_v1_errors_test( s ):
    print('[TEST] get_v1_errors_test')

    funcs = [invalid_ifmodsince, notmodified, forbidden, file_not_exist, no_start_slash, nohost, multihost]

    ## uris are already tested

    f = random.choice(funcs)

    return f(s)

def normal_get_req(s):
    print('[TEST] normal_get_req')

    uri = b'/index.html'

    req =  b'GET ' + uri +b' HTTP/1.1\r\n'
    req += b'Host: chess\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    f = open('/data/index.html', 'rb')
    data = f.read()
    f.close()

    if y != data:
        print('[ERROR] Failed to receive expected data')
        print('Expected: ')
        print(data)
        print('Received: ')
        print(y)
        return 1

    print('\t[SUCCESS] normal_get_req')

    return 0

def normal_get_with_params(s):
    print('[TEST] normal_get_with_params')

    uri = '/welcome.php'

    f = open('/data/welcome.php')

    data = f.read()

    f.close()

    name = randomstring(random.randint(5,10))

    email = randomstring(random.randint(5,10)) + '@' + randomstring(random.randint(5,10)) + '.com'

    header = '<?php $name = "%s"; $email = "%s"; ?>\n' %(name, email)

    data = header + data

    f = open('/data/temp.php', 'wb')
    f.write(data.encode('utf-8'))
    f.close()

    p = subprocess.Popen(['php', '/data/temp.php'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()

    os.remove('/data/temp.php')

    ## now add the things
    uri += '?name='
    uri += name;
    uri += '&email='
    uri += email

    for x in range(random.randint(1,3)):
        uri += '&%s=%s' %(randomstring(random.randint(5,10)),randomstring(random.randint(5,10)))


    req =  b'GET ' + uri.encode('utf-8') +b' HTTP/1.1\r\n'
    req += b'Host: chess\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    if y != stdout:
        print('[ERROR] Failed to receive expected data')
        print('Expected: ')
        print(stdout)
        print('Received: ')
        print(y)
        return 1

    print('\t[SUCCESS] normal_get_with_params')
    return 0

def get_range(s):
    print('[TEST] get_range')

    f = open('/data/index.html')
    data = f.read()
    f.close()


    uri = b'/index.html'

    req =  b'GET ' + uri +b' HTTP/1.1\r\n'
    req += b'Host: chess\r\n'

    expected = []

    header = b'Range: bytes='

    for x in range(random.randint(1,2)):
        start = random.randint(0, int(len(data)/2) )

        end = random.randint(int(len(data)/2) + 1, len(data))

        expected.append(data[start:end].encode('utf-8'))

        header += b'%d-%d,' %(start, end)

    header = header.rstrip(b',') + b'\r\n\r\n'

    req += header    

    print(req)

    SW(s, req)

    resp = RU(s, '\r\n\r\n').encode('utf-8')

    cl = resp.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = resp[cl + 16:].split(b'\r\n')

    length = int(d[0])

    content = s.recv(length)

    for x in expected:
        if content.find(x) == -1:
            print('[ERROR] Failed to receive expected data')
            print('Expected: ')
            print(x)
            print('Received: ')
            print(content)

            return 1

    print('\t[SUCCESS] get_range')

    return 0

def valid_get_v1(s):
    print('[TEST] valid_get_v1')

    funcs = [get_range, normal_get_with_params, normal_get_req]

    f = random.choice(funcs)
    
    return f(s)

def post_nohost( s ):
    print('[TEST] post_nohost')

    req =  b'POST /index.html HTTP/1.1\r\n'
    req += randomstring(5).encode('utf-8') + b': ' + randomstring(10).encode('utf-8') + b'\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Bad Request') == -1:
        return 1

    print('\t[SUCCESS] post_nohost')

    return 0

def post_multihost( s ):
    print('[TEST] post_multihost')

    req =  b'POST /index.html HTTP/1.1\r\n'
    req += b'Host: chess\r\n'
    req += b'Host: ' + randomstring(5).encode('utf-8') + b'\r\n'
    req += randomstring(5).encode('utf-8') + b': ' + randomstring(10).encode('utf-8') + b'\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Bad Request') == -1:
        return 1

    print('\t[SUCCESS] post_multihost')

    return 0

def post_no_start_slash( s ):
    print('[TEST] post_no_start_slash')

    req =  b'POST index.html HTTP/1.1\r\n'
    req += b'Host: chess\r\n'
    req += randomstring(5).encode('utf-8') + b': ' + randomstring(10).encode('utf-8') + b'\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Bad Request') == -1:
        return 1

    print('\t[SUCCESS] post_no_start_slash')

    return 0

def post_file_not_exist(s):
    print('[TEST] post_file_not_exist')

    uri = b'/' + randomstring(5).encode('utf-8') + b'.html'

    req =  b'POST ' + uri +b' HTTP/1.1\r\n'
    req += b'Host: chess\r\n'
    req += randomstring(5).encode('utf-8') + b': ' + randomstring(10).encode('utf-8') + b'\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Not Found') == -1:
        return 1

    print('\t[SUCCESS] post_file_not_exist')

    return 0

def post_forbidden(s):
    print('[TEST] post_forbidden')

    uri = b'/'

    req =  b'POST ' + uri +b' HTTP/1.1\r\n'
    req += b'Host: chess\r\n'
    req += randomstring(5).encode('utf-8') + b': ' + randomstring(10).encode('utf-8') + b'\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    print(y)

    if y.find(b'Message: Forbidden') == -1:
        return 1

    print('\t[SUCCESS] post_forbidden')

    return 0

def post_v1_errors_test( s ):
    print('[TEST] post_v1_errors_test')

    funcs = [post_forbidden, post_file_not_exist, post_no_start_slash, post_nohost, post_multihost]

    f = random.choice(funcs)

    return f(s)

def post_basic_v1(s):
    print('[TEST] post_basic_v1')

    uri = '/welcome.php'

    f = open('/data/welcome.php')

    data = f.read()

    f.close()

    name = randomstring(random.randint(5,10))

    email = randomstring(random.randint(5,10)) + '@' + randomstring(random.randint(5,10)) + '.com'

    header = '<?php $name = "%s"; $email = "%s"; ?>\n' %(name, email)

    data = header + data

    f = open('/data/temp.php', 'wb')
    f.write(data.encode('utf-8'))
    f.close()

    p = subprocess.Popen(['php', '/data/temp.php'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()

    os.remove('/data/temp.php')

    ## now add the things
    uri += '?name='
    uri += name;
    uri += '&email='
    uri += email

    for x in range(random.randint(1,3)):
        uri += '&%s=%s' %(randomstring(random.randint(5,10)),randomstring(random.randint(5,10)))


    req =  b'POST ' + uri.encode('utf-8') +b' HTTP/1.1\r\n'
    req += b'Host: chess\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    if y != stdout:
        print('[ERROR] Failed to receive expected data')
        print('Expected: ')
        print(stdout)
        print('Received: ')
        print(y)
        return 1

    print('\t[SUCCESS] post_basic_v1')

    return 0

def post_send_urlform_v1(s):
    print('[TEST] post_send_urlform_v1')

    uri = '/welcome.php'

    f = open('/data/welcome.php')

    data = f.read()

    f.close()

    name = randomstring(random.randint(5,10))

    email = randomstring(random.randint(5,10)) + '@' + randomstring(random.randint(5,10)) + '.com'

    header = '<?php $name = "%s"; $email = "%s"; ?>\n' %(name, email)

    data = header + data

    f = open('/data/temp.php', 'wb')
    f.write(data.encode('utf-8'))
    f.close()

    p = subprocess.Popen(['php', '/data/temp.php'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()

    os.remove('/data/temp.php')

    data = 'name=%s&email=%s' %(name, email)

    for x in range(random.randint(1,3)):
        data += '&%s=%s' %(randomstring(random.randint(5,10)),randomstring(random.randint(5,10)))


    req =  b'POST ' + uri.encode('utf-8') +b' HTTP/1.1\r\n'
    req += b'Content-Type: application/x-www-form-urlencoded\r\n'
    req += b'Content-Length: %d\r\n' %(len(data))
    req += b'Host: chess\r\n\r\n'
    req += data

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    if y != stdout:
        print('[ERROR] Failed to receive expected data')
        print('Expected: ')
        print(stdout)
        print('Received: ')
        print(y)
        return 1


    print('\t[SUCCESS] post_send_urlform_v1')

    return 0

def post_send_multi_part_v1(s):
    print('[TEST] post_send_multi_part_v1')

    uri = '/welcome.php'

    f = open('/data/welcome.php')

    data = f.read().encode('utf-8')

    f.close()

    name = randomstring(random.randint(5,10)).encode('utf-8')

    email = randomstring(random.randint(5,10)) + '@' + randomstring(random.randint(5,10)) + '.com'

    email = email.encode('utf-8')

    header = b'<?php $name = "%s"; $email = "%s"; ?>\n' %(name, email)

    data = header + data

    f = open('/data/temp.php', 'wb')
    f.write(data)
    f.close()

    p = subprocess.Popen(['php', '/data/temp.php'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()

    os.remove('/data/temp.php')

    boundary = b'-'*20 + randomstring(20).encode('utf-8')

    data = b'--%s\r\n' %(boundary)
    data += b'Content-Disposition: form-data; name="name"\r\n\r\n'
    data += b'%s\r\n' %(name)
    data += b'--%s\r\n' %(boundary)
    data += b'Content-Disposition: form-data; name="email"\r\n\r\n'
    data += b'%s\r\n' %(email)
    data += b'--%s--' %(boundary)

    req =  b'POST ' + uri.encode('utf-8') +b' HTTP/1.1\r\n'
    req += b'Content-Type: multipart/form-data; boundary=%s\r\n' %(boundary)
    req += b'Content-Length: %d\r\n' %(len(data))
    req += b'Host: chess\r\n\r\n'

    SW(s, req)

    SW(s, data)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    if y != stdout:
        print('[ERROR] Failed to receive expected data')
        print('Expected: ')
        print(stdout)
        print('Received: ')
        print(y)
        return 1


    print('\t[SUCCESS] post_send_multi_part_v1')

    return 0

def post_upload_file_v1(s):
    print('[TEST] post_upload_file_v1')

    uri = '/file-upload-manager.php'

    f = open('/data/file-upload-manager.php')

    data = f.read().encode('utf-8')

    f.close()

    name = randomstring(random.randint(5,10)).encode('utf-8')

    filedata = randomstring(random.randint(200,500)).encode('utf-8')

    header = b'<?php $_SERVER["REQUEST_METHOD"] = "POST";\n'
    header += b'$_FILES["chessfile"]["name"] = "%s";\n' %(name)
    header += b'$_FILES["chessfile"]["tmp_name"] = "/tmp/chess_x2Wuqj";\n'
    header += b'$_FILES["chessfile"]["size"] = %d;\n' %(len(filedata))
    header += b'$_FILES["chessfile"]["error"] = 0;\n'
    header += b'$_FILES["chessfile"]["type"] = "application/octet-stream";\n'
    header += b'?>\n'

    data = header + data

    f = open('/data/temp.php', 'wb')
    f.write(data)
    f.close()

    p = subprocess.Popen(['php', '/data/temp.php'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = p.communicate()

    os.remove('/data/temp.php')

    boundary = b'-'*20 + randomstring(20).encode('utf-8')

    data = b'--%s\r\n' %(boundary)
    data += b'Content-Disposition: form-data; name="chessfile"; filename=%s\r\n\r\n' %(name)
    data += b'%s\r\n' %(filedata)
    data += b'--%s--' %(boundary)

    req =  b'POST ' + uri.encode('utf-8') +b' HTTP/1.1\r\n'
    req += b'Content-Type: multipart/form-data; boundary=%s\r\n' %(boundary)
    req += b'Content-Length: %d\r\n' %(len(data))
    req += b'Host: chess\r\n\r\n'

    SW(s, req)

    SW(s, data)

    print("DATA: ")
    print(data)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    cl = y.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = y[cl + 16:].split(b'\r\n')

    length = int(d[0])

    y = s.recv(length)

    start = stdout.find(b'Temporary Name: ')

    if y[:start] != stdout[:start]:
        print('[ERROR] Failed to receive expected data')
        print('Expected: ')
        print(stdout)
        print('Received: ')
        print(y)
        return 1


    print('\t[SUCCESS] post_upload_file_v1')

    return 0

def post_method_v1(s):
    print('[TEST] post_method_v1')

    funcs = [post_upload_file_v1, post_send_multi_part_v1, post_send_urlform_v1, post_basic_v1]

    funcs = [post_upload_file_v1]

    f = random.choice(funcs)

    return f(s)

def put_method_v1(s):
    print('[TEST] put_method_v1')


    uri = '/' + randomstring(8) + '.html'

    file = '<html><head><title>' + randomstring(8) + '</title></head>'
    file += '<body>'
    file += randomstring(20) #random.randint(50,100))
    file += '</body></html>'

    file = file.encode('utf-8')

    boundary = b'-'*20 + randomstring(20).encode('utf-8')

    name = randomstring(8).encode('utf-8')

    data = b'--%s\r\n' %(boundary)
    data += b'Content-Disposition: form-data; name="chessfile"; filename="%s"\r\n\r\n' %(name)
    data += b'%s\r\n' %(file)
    data += b'--%s--\r\n' %(boundary)

    req =  b'PUT ' + uri.encode('utf-8') +b' HTTP/1.1\r\n'
    req += b'Content-Type: multipart/form-data; boundary=%s\r\n' %(boundary)
    req += b'Content-Length: %d\r\n' %(len(data))
    req += b'Host: chess\r\n\r\n'

    SW(s, req)

    SW(s, data)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    if y.find(b'201 Created') == -1:
        print('\t[FAIL] Did not receive 201 Created')
        return 1

    req = b'GET ' + uri.encode('utf-8') +b' HTTP/1.1\r\n'
    req += b'Host: chess\r\n\r\n'

    SW(s, req)

    resp = RU(s, '\r\n\r\n').encode('utf-8')

    cl = resp.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = resp[cl + 16:].split(b'\r\n')

    length = int(d[0])

    content = s.recv(length)

    if content != file:
        print('\t[FAIL] Expected: %s' %(file))
        print('Received: %s' %(content) )
        return 1

    print('\t[SUCCESS] post_upload_file_v1')

    return 0

def delete_method_v1(s):
    print('[TEST] delete_method_v1')


    uri = '/' + randomstring(8) + '.html'

    file = '<html><head><title>' + randomstring(8) + '</title></head>'
    file += '<body>'
    file += randomstring(20) #random.randint(50,100))
    file += '</body></html>'

    file = file.encode('utf-8')

    boundary = b'-'*20 + randomstring(20).encode('utf-8')

    name = randomstring(8).encode('utf-8')

    data = b'--%s\r\n' %(boundary)
    data += b'Content-Disposition: form-data; name="chessfile"; filename="%s"\r\n\r\n' %(name)
    data += b'%s\r\n' %(file)
    data += b'--%s--\r\n' %(boundary)

    req =  b'PUT ' + uri.encode('utf-8') +b' HTTP/1.1\r\n'
    req += b'Content-Type: multipart/form-data; boundary=%s\r\n' %(boundary)
    req += b'Content-Length: %d\r\n' %(len(data))
    req += b'Host: chess\r\n\r\n'

    SW(s, req)

    SW(s, data)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    if y.find(b'201 Created') == -1:
        print('\t[FAIL] Did not receive 201 Created')
        return 1

    req = b'GET ' + uri.encode('utf-8') +b' HTTP/1.1\r\n'
    req += b'Host: chess\r\n\r\n'

    SW(s, req)

    resp = RU(s, '\r\n\r\n').encode('utf-8')

    cl = resp.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = resp[cl + 16:].split(b'\r\n')

    length = int(d[0])

    content = s.recv(length)

    if content != file:
        print('\t[FAIL] Expected: %s' %(file))
        print('Received: %s' %(content) )
        return 1

    req = b'DELETE ' + uri.encode('utf-8') +b' HTTP/1.1\r\n'
    req += b'Host: chess\r\n\r\n'

    SW(s, req)

    y = RU(s, '\r\n\r\n').encode('utf-8')

    if y.find(b'204 No Content') == -1:
        print('\t[FAIL] Did not receive 201 Created')
        return 1

    req = b'GET ' + uri.encode('utf-8') +b' HTTP/1.1\r\n'
    req += b'Host: chess\r\n\r\n'

    SW(s, req)

    resp = RU(s, '\r\n\r\n').encode('utf-8')

    cl = resp.find(b'Content-Length: ')

    if ( cl == -1):
        return 1

    d = resp[cl + 16:].split(b'\r\n')

    length = int(d[0])

    content = s.recv(length)

    if content.find(b'Not Found') == -1:
        print('\t[FAIL] Did not receive expected response')
        return 1

    print('\t[SUCCESS] delete_method_v1')

    return 0

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

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('[INFO] SEED: {0}'.format(SEED))

    random.seed( SEED )

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(10)

    try:
        s = connect( HOST, PORT )
    except:
        print('[ERROR] Failed to connect to: %s:%d' %(HOST, PORT))
        exit(1)

    funcs = [delete_method_v1, put_method_v1, post_method_v1, post_v1_errors_test, valid_get_v1, parse_errors_test, get_v1_errors_test ]

    #funcs = [delete_method_v1]

    for i in range(0,50):
        d = random.choice( funcs )

        if d( s ):
            print('\t[FAIL] Unsuccessful test')
            exit(1)

if __name__ == '__main__':
    main()
