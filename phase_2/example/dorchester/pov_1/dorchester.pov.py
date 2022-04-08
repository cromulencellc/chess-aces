from pwn import *
import sys
import os
import requests

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

    x = requests.post(f'http://{HOST}:{PORT}/signup.html', data={'user': 'chess', 'pass': 'pwned'}, allow_redirects=False)

    x = requests.post(f'http://{HOST}:{PORT}/login.html', data={'user': 'chess', 'pass': 'pwned'}, allow_redirects=False)

    if 'Set-Cookie' not in x.headers:
        print('[FAIL] Login failed: Did not find Set-Cookie')
        return 1

    cookie = x.headers['Set-Cookie']

    c = None

    if cookie.find(';') != -1:
        cookie = cookie.split(';')

        for l in cookie:
            if l.find('mgs') != -1:
                c = l
    else:
        c = cookie

    if c == None:
        print('[FAIL] Did not receive the mgs cookie')
        return 1

    c = c.split('=')

    print(f'[INFO] Cookie: {c}')

    x = requests.get(f'http://{HOST}:{PORT}/token', cookies = {c[0]: c[1]}, allow_redirects=False)

    print(f'[TOKEN] {x.text}')

    return 0
if __name__=='__main__':
    sys.exit(main())
