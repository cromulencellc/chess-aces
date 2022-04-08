from pwn import *

p = remote('localhost', 9999)

data = b'CONNECT 10.0.2.15:8888 HTTP/1.1\r\n'
data += b'Host: chess.com\r\n'
data += b'Proxy-Authorization: basic aGVsbG86d29ybGQ=\r\n\r\n'

p.send(data)

y = p.readuntil(b'\r\n\r\n')

print(y)

p.close()
