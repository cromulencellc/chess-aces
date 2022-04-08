import sys
from pwn import *
import requests

import random
import time
import string
import base64
import os
import datetime
import string
import signal

users = {}
cookies = {}

db_keys = {}

def handler( signum, frame):
    print('[FAIL] sigalrm fired Poller failed')
    exit(1)

def randomstring(l):
    q = ''

    for _ in range(l):
        q += random.choice(string.ascii_lowercase)

    return q

def create_db_key(host, port):
    global db_keys

    print('[TEST] create_db_key')

    k = randomstring(5)
    v = randomstring(5)

    url = f'http://{host}:{port}/api/v1/{k}'

    x = requests.put(url, data = {'value': v})

    if x.status_code == 200:
        print(f'\t[SUCCESS]: {k}')

        db_keys[k] = v
        return 0

    print('\t[FAIL]')

    return 1

def get_db_key(host, port):
    global db_keys

    if len(db_keys) == 0:
        if create_db_key(host, port):
            return 1

    print('[TEST] get_db_key')

    k = random.choice(list(db_keys.keys()))
    v = db_keys[k]

    url = f'http://{host}:{port}/api/v1/{k}'

    x = requests.get(url)

    if x.status_code == 200 and x.text == v:
        print(f'\t[SUCCESS]: {k}:{v}')
        return 0

    print('\t[FAIL]')

    return 1

def get_db_key_not_there(host, port):
    print('[TEST] get_db_key_not_there')

    k = randomstring(10)

    url = f'http://{host}:{port}/api/v1/{k}'

    x = requests.get(url)

    if x.status_code == 404:
        print('\t[SUCCESS]')
        return 0

    print('\t[FAIL]')

    return 1

def delete_db_key(host, port):
    global db_keys

    if len(db_keys) == 0:
        if create_db_key(host, port):
            return 1

    print('[TEST] delete_db_key')

    k = random.choice(list(db_keys.keys()))

    url = f'http://{host}:{port}/api/v1/{k}'

    x = requests.delete(url)

    if x.status_code == 200:
        db_keys.pop(k)
        print('\t[SUCCESS]')
        return 0

    print('\t[FAIL]')

    return 1

def delete_db_key_not_there(host, port):
    print('[TEST] delete_db_key_not_there')

    k = randomstring(10)

    url = f'http://{host}:{port}/api/v1/{k}'

    x = requests.get(url)

    if x.status_code == 404:
        print('\t[SUCCESS]')
        return 0

    print('\t[FAIL]')

    return 1

def login_no_user( host, port ):
    print('[TEST] login_no_user')

    x = requests.post(f'http://{host}:{port}/login.html', data={'pass': randomstring(5)})

    if x.text.find('user, pass required.') != -1:
        print('\t[SUCCESS]')
        return 0

    print('\t[FAILED]')
    return 1

def login_no_pass( host, port ):
    print('[TEST] login_no_pass')

    x = requests.post(f'http://{host}:{port}/signup.html', data={'user': randomstring(5)})

    if x.text.find('user, pass required.') != -1:
        print('\t[SUCCESS]')
        return 0

    print('\t[FAILED]')
    return 1

def delete_oldest_cookie():
    global cookies

    oldest_key = None

    for x in cookies:
        if oldest_key == None:
            oldest_key = x
        else:
            if cookies[x] < cookies[oldest_key]:
                oldest_key = x

    cookies.pop(oldest_key)

def login_user( host, port ):
    global users
    global cookies

    ## No more than 10 logins
    if len(cookies) == 10:
        delete_oldest_cookie()

    if len( users ) == 0:
        if ( signup_user( host, port ) ):
            return 1

    print('[TEST] login_user')

    u = random.choice( list(users.keys()))
    p = users[u]

    x = requests.post(f'http://{host}:{port}/login.html', data={'user': u, 'pass': p}, allow_redirects=False)

    if 'Set-Cookie' not in x.headers:
        print('\t[FAILED]')
        return 1

    cookies[u] = [ x.headers['Set-Cookie'], time.time() ]

    print('\t[SUCCESS]')
    return 0

def check_token_not_logged_in( host, port ):
    print('[TEST] check_token_not_logged_in')

    x = requests.get(f'http://{host}:{port}/token')

    if (x.text.find('Please log in') != -1):
        print('\t[SUCCESS]')
        return 0

    print('\t[FAILED]')
    return 1

def check_token_logged_in( host, port ):
    global cookies
    global users

    # pick a cookie
    if len(cookies) == 0:
        if ( login_user(host, port) ):
            return 1

    print('[TEST] check_token_logged_in')

    u = random.choice( list( cookies.keys()))
    p = users[u]
    c = cookies[u][0].split('=')[1]

    x = requests.get(f'http://{host}:{port}/token', cookies = {"mgs": c})

    if (x.status_code == 403):
        print('\t[SUCCESS]')
        return 0

    print('\t[FAILED]')
    return 1

def get_invalid_random_nl( host, port ):
    print('[TEST] get_invalid_random_nl')

    x = requests.get(f'http://{host}:{port}/{randomstring(5)}')

    if (x.text.find('Please log in') != -1):
        print('\t[SUCCESS]')
        return 0

    print('\t[FAILED]')
    return 1

def signup_no_user( host, port ):
    print('[TEST] signup_no_user')

    x = requests.post(f'http://{host}:{port}/signup.html', data={'pass': randomstring(5)})

    if x.text.find('user, pass required.') != -1:
        print('\t[SUCCESS]')
        return 0

    print('\t[FAILED]')
    return 1

def signup_no_pass( host, port ):
    print('[TEST] signup_no_pass')

    x = requests.post(f'http://{host}:{port}/signup.html', data={'user': randomstring(5)})

    if x.text.find('user, pass required.') != -1:
        print('\t[SUCCESS]')
        return 0

    print('\t[FAILED]')
    return 1

def signup_user( host, port ):
    global users

    u = randomstring(5)
    p = randomstring(5)

    print('[TEST] signup_user')

    x = requests.post(f'http://{host}:{port}/signup.html', data={'user': u, 'pass': p})

    if x.text.find('Please log in') != -1:
        print('\t[SUCCESS]')
        users[u] = p
        return 0

    print('\t[FAILED]')
    return 1

def logout_user_nl( host, port ):
    print('[TEST] logout_user_nl')

    x = requests.get(f'http://{host}:{port}/logout', cookies = {"mgs": randomstring(10)}, allow_redirects=False)

    if x.status_code == 400:
        print('\t[SUCCESS]')
        return 0

    print('\t[FAILED]')
    return 1

def logout_user( host, port ):
    global cookies
    global users

    # pick a cookie
    if len(cookies) == 0:
        if ( login_user(host, port) ):
            return 1

    print('[TEST] logout_user')

    u = random.choice( list( cookies.keys()))
    p = users[u]
    c = cookies[u][0].split('=')[1]

    x = requests.get(f'http://{host}:{port}/index.shtml', cookies = {"mgs": c})

    if (x.text.find('lucky number') != -1):
        print('\t[SUCCESS]')
        return 0

    print('\t[FAILED]')
    return 1

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

    for i in range(random.randint(15,25)):
        d = random.choice( [create_db_key, get_db_key, get_db_key_not_there, delete_db_key, logout_user, logout_user_nl, signup_user, signup_no_pass, signup_no_user, get_invalid_random_nl, check_token_logged_in, check_token_not_logged_in, login_user, login_no_pass, login_no_user, delete_db_key_not_there] )

        if d( HOST, PORT ):
            exit(1)

if __name__ == '__main__':
    main()
