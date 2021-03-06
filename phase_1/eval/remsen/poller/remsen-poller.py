import sys
import socket
import random
import time
import string
import base64
import os
import datetime
import string

pwd = ''
homedir = ''

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

def readuntil( s, u ):
    z = ''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1).decode('utf-8')
    except Exception as e:
        print('Failed to receive data')
        print(e)
        return None

    return z

def readline( s ):
    return readuntil( s, '\n')

def send_wrapper(s, data):
    #print data
    s.send(data.encode('utf-8'))

def getbanner(s):
    y = readuntil(s, "\r\n")

    if y.find( "220 chessFTPd (.0.0.0.0.0.0.1)") == - 1:
        return 1

    return 0

def login(s):
    global pwd
    global homedir

    print('[TEST] login')

    send_wrapper(s, "USER guest\n")

    y = readuntil(s, "\r\n")

    if y.find("331 Please specify the password") == -1:
        return 1

    send_wrapper(s, "PASS guest\n")

    y = readuntil(s, "\r\n")

    if y.find("230 Login successful.") == -1:
        return 1

    send_wrapper(s, 'PWD\n')

    y = readuntil(s, '\r\n')

    if y.find('is the current directory') == -1:
        print('[ERROR] login() Failed to get directory')
        return 1

    ## 257 "/tmp" is the current directory

    f = y.split(' ')[1][1:-1]

    pwd = f
    homedir = pwd

    print('[INFO] Current directory: %s' %pwd)

    return 0

def handleLIST(s):
    global homedir

    print('[TEST] handleLIST')
    listfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    host = s.getsockname()[0]
    port = random.randint(4000, 20000)

    useport = (random.randint(0,100) > 50)

    d = homedir

    if useport:
        print('\tUsing PORT')

        listfd.bind((host, port))
        listfd.listen(1)

        req = "PORT " + str(host).replace('.', ',') + ',' + str(int(port/256)) + ',' + str(port % 256) + "\n"

        send_wrapper(s, req)

        y = readuntil(s, '\r\n')

        if y.find("200 PORT command successful. Consider using PASV.") == -1:
            print('[ERROR] handleLIST() port command failed: %s' %y)

            listfd.close()
            return 1
    else:
        print('\tUsing PASV')
        send_wrapper(s, "PASV\r\n")

        y = readuntil(s, "\r\n")

        if y.find("227 Entering Passive Mode") == -1:
            print('[ERROR] handleLIST() pasv command failed: %s' %y)
            return 1

        z = y.split(' ')[4]

        print(z)

        x = z.split(',')

        ip = x[0][1:] + '.' + x[1] + '.' + x[2] + '.' + x[3]
        port = int(x[4]) * 256 + int(x[5][:-4])

        print("IP: %s port: %d" %(ip, port))

        try:
            listfd.connect((ip, port))
        except:
            print('[ERROR] handleLIST() failed to connect to %s:%d' %(ip, port))
            return 1

    
    send_wrapper(s, "LIST " + d + "\n")

    if useport:
        c, addr = listfd.accept()
        listfd.close()
        listfd = c

    y = readuntil(s, '\r\n')

    if y.find("150 Here comes the directory listing.\r\n") == -1:
        print('[ERROR] handleLIST() unexpected: %s' %y)
        return 1

    b = ''
    r = ''
    try:
        listfd.settimeout( .1 )
        r = listfd.recv(1024)
        while r != '':
            b += r
            r = listfd.recv(1024)
    except:
        pass

    print(b)
    listfd.close()

    y = readuntil(s, '\r\n')

    if y.find('226 Directory send OK.') == -1:
        print('[ERROR] handleLIST() failed: %s' %(y))
        return 1

    return 0

def handleNI(s):
    z = random.choice(['ABOR', 'ACCT', 'ALLO', 'EPRT', 'EPSV', 'OPTS', 'STAT', 'FEAT', 'SIZE', 'SITE'])
    send_wrapper(s, z + '\n')

    y = readuntil(s, '\r\n')

    if y.find('520 Not implemented') == -1:
        return 1

    return 0

def handleNLST(s):
    global homedir

    print('[TEST] handleNLST')
    listfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    host = s.getsockname()[0]
    port = random.randint(4000, 20000)

    useport = (random.randint(0,100) > 0)

    d = homedir

    if useport:
        print('\tUsing PORT')

        listfd.bind((host, port))
        listfd.listen(1)

        req = "PORT " + str(host).replace('.', ',') + ',' + str(int(port/256)) + ',' + str(port % 256) + "\n"

        send_wrapper(s, req)

        y = readuntil(s, '\r\n')

        if y.find("200 PORT command successful. Consider using PASV.") == -1:
            print('[ERROR] handleNLST() port command failed: %s' %y)

            listfd.close()
            return 1
    else:
        print('\tUsing PASV')
        send_wrapper(s, "PASV\r\n")

        y = readuntil(s, "\r\n")

        if y.find("227 Entering Passive Mode") == -1:
            print('[ERROR] handleNLST() pasv command failed: %s' %y)
            return 1

        z = y.split(' ')[4]

        print(z)

        x = z.split(',')

        ip = x[0][1:] + '.' + x[1] + '.' + x[2] + '.' + x[3]
        port = int(x[4]) * 256 + int(x[5][:-4])

        print("IP: %s port: %d" %(ip, port))

        try:
            listfd.connect((ip, port))
        except:
            print('[ERROR] handleNLST() failed to connect to %s:%d' %(ip, port))
            return 1

    
    send_wrapper(s, "NLST " + d + "\n")

    if useport:
        c, addr = listfd.accept()
        listfd.close()
        listfd = c

    y = readuntil(s, '\r\n')

    if y.find("150 Here comes the directory listing.\r\n") == -1:
        print('[ERROR] handleNLST() unexpected: %s' %y)
        return 1

    b = ''
    r = ''
    try:
        listfd.settimeout( .1 )
        r = listfd.recv(1024)
        while r != '':
            b += r
            r = listfd.recv(1024)
    except:
        pass

    print (b)
    listfd.close()

    y = readuntil(s, '\r\n')

    if y.find('226 Directory send OK.') == -1:
        print('[ERROR] handleNLST() unexpected: %s' %y)
        return 1

    return 0

def handleSYST(s):
    print('[TEST] SYST')

    send_wrapper(s, 'SYST\n')

    y = readuntil(s, '\r\n')

    if y.find('215 UNIX Type: L8') == -1:
        print('[ERROR] handleSYST() command failed: %s' %y)
        return 1

    return 0

def handleNOOP(s):
    print('[TEST] NOOP')

    send_wrapper(s, 'NOOP\n')

    y = readuntil(s, '\r\n')

    if y.find('200 NOOP ok.') == -1:
        print('[ERROR] handleNOOP() command failed: %s' %y)
        return 1

    return 0

def handlePWD( s ):
    global pwd

    print('[TEST] PWD')

    send_wrapper(s, 'PWD\n')

    y = readuntil(s, '\r\n')

    if y.find('257 "%s" is the current directory' %(pwd)) == -1:
        print('[ERROR] handlePWD() command failed: %s' %y)
        return 1

    return 0

def handleXPWD( s ):
    global pwd

    print('[TEST] XPWD')

    send_wrapper(s, 'XPWD\n')

    y = readuntil(s, '\r\n')

    if y.find('257 "%s" is the current directory' %(pwd)) == -1:
        print('[ERROR] handleXPWD() command failed: %s' %y)
        return 1

    return 0

def handleSTRU( s ):
    print('[TEST] STRU')

    send_wrapper(s, 'STRU f\n')

    y = readuntil(s, '\r\n')

    if y.find('200 Structure set to F.') == -1:
        print('[ERROR] handleSTRU() command failed: %s' %y)
        return 1

    return 0

def handleCWD( s ):
    global homedir
    global pwd

    print('[TEST] CWD')

    send_wrapper(s, 'CWD %s\n' %(homedir))

    y = readuntil(s, '\r\n')

    if y.find('250 Directory successfully changed.') == -1:
        print('[ERROR] handleCWD() command failed: %s' %y)
        return 1

    pwd = homedir

    return 0

def handleXCWD( s ):
    global homedir
    global pwd

    print('[TEST] XCWD')

    send_wrapper(s, 'XCWD %s\n' %(homedir))

    y = readuntil(s, '\r\n')

    if y.find('250 Directory successfully changed.') == -1:
        print('[ERROR] handleXCWD() command failed: %s' %y)
        return 1

    pwd = homedir

    return 0

def handleMKDRMD( s ):
    global homedir

    print('[TEST] MKD / RMD')

    l = randomstring( random.randint(8,70) )

    newdir = homedir + "/" + l

    send_wrapper(s, 'MKD %s\n' %(newdir))

    y = readuntil(s, '\r\n')

    if y.find('257 "%s" created' %newdir) == -1:
        print('[ERROR] handleMKD() command failed: %s' %y)
        return 1

    send_wrapper(s, 'RMD %s\n' %(newdir) )

    y = readuntil(s, '\r\n')

    if y.find('250 Remove directory operation successful.') == -1:
        print('[ERROR] handleMKD() command failed: %s' %y)
        return 1

    return 0

def handleCWDCDUPMKDRMD( s ):
    global homedir

    print('[TEST] MKD / CWD / CDUP / RMD')

    l = randomstring( random.randint(8,70) )

    newdir = homedir + "/" + l

    send_wrapper(s, 'MKD %s\n' %(newdir))

    y = readuntil(s, '\r\n')

    if y.find('257 "%s" created' %newdir) == -1:
        print('[ERROR] handleCWDCDUPMKDRMD() command failed: %s' %y)
        return 1

    send_wrapper(s, 'CWD %s\n' %(l) )

    y = readuntil(s, '\r\n')

    if y.find('250 Directory successfully changed.') == -1:
        print('[ERROR] handleCWDCDUPMKDRMD() command failed: %s' %y)
        return 1

    send_wrapper(s, 'CDUP\n')

    y = readuntil(s, '\r\n')

    if y.find('250 Directory successfully changed.') == -1:
        print('[ERROR] handleCWDCDUPMKDRMD() command failed: %s' %y)
        return 1

    send_wrapper(s, 'RMD %s\n' %(newdir) )

    y = readuntil(s, '\r\n')

    if y.find('250 Remove directory operation successful.') == -1:
        print('[ERROR] handleCWDCDUPMKDRMD() command failed: %s' %y)
        return 1

    return 0

def handleXMKDXRMD( s ):
    global homedir

    print('[TEST] XMKD / XRMD')

    l = randomstring( random.randint(8,70) )

    newdir = homedir + "/" + l

    send_wrapper(s, 'XMKD %s\n' %(newdir))

    y = readuntil(s, '\r\n')

    if y.find('257 "%s" created' %newdir) == -1:
        print('[ERROR] handleXMKD() command failed: %s' %y)
        return 1

    send_wrapper(s, 'XRMD %s\n' %(newdir) )

    y = readuntil(s, '\r\n')

    if y.find('250 Remove directory operation successful.') == -1:
        print('[ERROR] handleXMKD() command failed: %s' %y)
        return 1

    return 0

def handleXCWDCDUPMKDRMD( s ):
    global homedir

    print('[TEST] XMKD / XCWD / XCDUP / XRMD')

    l = randomstring( random.randint(8,70) )

    newdir = homedir + "/" + l

    send_wrapper(s, 'XMKD %s\n' %(newdir))

    y = readuntil(s, '\r\n')

    if y.find('257 "%s" created' %newdir) == -1:
        print('[ERROR] handleXCWDCDUPMKDRMD() command failed: %s' %y)
        return 1

    send_wrapper(s, 'XCWD %s\n' %(l) )

    y = readuntil(s, '\r\n')

    if y.find('250 Directory successfully changed.') == -1:
        print('[ERROR] handleXCWDCDUPMKDRMD() command failed: %s' %y)
        return 1

    send_wrapper(s, 'XCUP\n')

    y = readuntil(s, '\r\n')

    if y.find('250 Directory successfully changed.') == -1:
        print('[ERROR] handleXCWDCDUPMKDRMD() command failed: %s' %y)
        return 1

    send_wrapper(s, 'XRMD %s\n' %(newdir) )

    y = readuntil(s, '\r\n')

    if y.find('250 Remove directory operation successful.') == -1:
        print('[ERROR] handleXCWDCDUPMKDRMD() command failed: %s' %y)
        return 1

    return 0

def handleMDTM( s ):
    global homedir

    print('[TEST] MDTM')

    send_wrapper(s, 'MDTM %s/testfile\n' %(homedir))

    y = readuntil(s, '\r\n')

    if y.find('213 ') == -1:
        print('[ERROR] handleMDTM() command failed: %s' %y)
        return 1

    return 0

def handleRNFRRNTO( s ):
    global homedir

    print('[TEST] RNFR / RNTO')

    send_wrapper(s, 'RNFR %s/testfile\n' %(homedir))

    y = readuntil(s, '\r\n')

    if y.find('350 Ready for RNTO.') == -1:
        print('[ERROR] handleRNFRRNTO() command failed: %s' %y)
        return 1

    l = randomstring( random.randint(8,70) )

    send_wrapper(s, 'RNTO %s/%s\n' %(homedir, l) )

    y = readuntil(s, '\r\n')

    if y.find('250 Rename successful.') == -1:
        print('[ERROR] handleRNFRRNTO() command failed: %s' %y)
        return 1

    send_wrapper(s, 'RNFR %s/%s\n' %(homedir, l))

    y = readuntil(s, '\r\n')

    if y.find('350 Ready for RNTO.') == -1:
        print('[ERROR] handleRNFRRNTO() command failed: %s' %y)
        return 1

    send_wrapper(s, 'RNTO %s/testfile\n' %(homedir) )

    y = readuntil(s, '\r\n')

    if y.find('250 Rename successful.') == -1:
        print('[ERROR] handleRNFRRNTO() command failed: %s' %y)
        return 1

    return 0

def handleRETR( s ):
    global homedir
    print('[TEST] handleRETR')

    retrfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    host = s.getsockname()[0]
    port = random.randint(4000, 20000)

    useport = (random.randint(0,100) > 50)

    if useport:
        print('\tUsing PORT')

        retrfd.bind((host, port))
        retrfd.listen(1)

        req = "PORT " + str(host).replace('.', ',') + ',' + str(int(port/256)) + ',' + str(port % 256) + "\n"

        send_wrapper(s, req)

        y = readuntil(s, '\r\n')

        if y.find("200 PORT command successful. Consider using PASV.") == -1:
            print('[ERROR] handleRETR() port command failed: %s' %y)

            retrfd.close()
            return 1
    else:
        print('\tUsing PASV')
        send_wrapper(s, "PASV\r\n")

        y = readuntil(s, "\r\n")

        if y.find("227 Entering Passive Mode") == -1:
            print('[ERROR] handleRETR() pasv command failed: %s' %y)
            return 1

        z = y.split(' ')[4]

        print(z)

        x = z.split(',')

        ip = x[0][1:] + '.' + x[1] + '.' + x[2] + '.' + x[3]
        port = int(x[4]) * 256 + int(x[5][:-4])

        print("IP: %s port: %d" %(ip, port))

        try:
            retrfd.connect((ip, port))
        except:
            print('[ERROR] handleRETR() failed to connect to %s:%d' %(ip, port))
            return 1


    send_wrapper(s, 'TYPE I\n')

    y = readuntil(s, '\r\n')

    if y.find("200 Switching to BINARY mode.") == -1:
        print('[ERROR] handleRETR() type command failed: %s' %y)
        return 1

    send_wrapper(s, 'RETR %s/testfile\n' %(homedir))

    y = readuntil(s, '\r\n')

    if y.find("150 Opening BINARY mode data connection") == -1:
        print('[ERROR] handleRETR() command failed: %s' %y)
        return 1

    toread = int(y.split(' ')[8][1:-1])

    if useport:
        c, addr = retrfd.accept()
        retrfd.close()
        retrfd = c

    b = ''
    r = ''
    try:
        print('[INFO] handleRETR() getting %d bytes' %(toread))
        retrfd.settimeout( .1 )
        r = retrfd.recv(toread)
    except:
        pass

    retrfd.close()

    print('[INFO] Retrieved the bytes')
    
    y = readuntil(s, '\r\n')

    if y.find('226 Transfer complete.') == -1:
        print('[ERROR] handleRETR() failed: %s' %(y))
        return 1

    return 0

def handleSTOR( s ):
    global homedir
    print('[TEST] handleSTOR')

    storfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    host = s.getsockname()[0]
    port = random.randint(4000, 20000)

    useport = (random.randint(0,100) > 50)

    if useport:
        print('\tUsing PORT')

        storfd.bind((host, port))
        storfd.listen(1)

        req = "PORT " + str(host).replace('.', ',') + ',' + str(int(port/256)) + ',' + str(port % 256) + "\n"

        print(port)
        print(req)

        send_wrapper(s, req)

        y = readuntil(s, '\r\n')

        if y.find("200 PORT command successful. Consider using PASV.") == -1:
            print('[ERROR] handleSTOR() port command failed: %s' %y)

            storfd.close()
            return 1
    else:
        print('\tUsing PASV')
        send_wrapper(s, "PASV\r\n")

        y = readuntil(s, "\r\n")

        if y.find("227 Entering Passive Mode") == -1:
            print('[ERROR] handleSTOR() pasv command failed: %s' %y)
            return 1

        z = y.split(' ')[4]

        print(z)

        x = z.split(',')

        ip = x[0][1:] + '.' + x[1] + '.' + x[2] + '.' + x[3]
        port = int(x[4]) * 256 + int(x[5][:-4])

        print("IP: %s port: %d" %(ip, port))

        try:
            storfd.connect((ip, port))
        except:
            print('[ERROR] handleSTOR() failed to connect to %s:%d' %(ip, port))
            return 1


    send_wrapper(s, 'TYPE I\n')

    y = readuntil(s, '\r\n')

    if y.find("200 Switching to BINARY mode.") == -1:
        print('[ERROR] handleSTOR() type command failed: %s' %y)
        return 1

    try:
        f = open('testfile', 'rb')
    except:
        print('[ERROR] handleSTOR() failed to open testfile.')
        return 1

    filedata = f.read()
    f.close()

    nm = randomstring( random.randint(8,70) )

    send_wrapper(s, 'STOR %s/%s\n' %(homedir, nm))

    y = readuntil(s, '\r\n')

    if y.find("150 Ok to send data.") == -1:
        print('[ERROR] handleSTOR() command failed: %s' %y)
        return 1

    if useport:
        c, addr = storfd.accept()
        storfd.close()
        storfd = c

    b = ''
    r = ''
    try:
        print('[INFO] handleSTOR() sending data')
        storfd.settimeout( .1 )
        storfd.send(filedata)
    except:
        pass

    storfd.close()

    print('[INFO] Sent the bytes')
    
    y = readuntil(s, '\r\n')

    if y.find('226 Transfer complete.') == -1:
        print('[ERROR] handleSTOR() failed: %s' %(y))
        return 1

    send_wrapper(s, "DELE %s/%s\n" %(homedir, nm))

    y = readuntil(s, '\r\n')

    if y.find('250 Delete operation successful.') == -1:
        print('[ERROR] DELE failed: %s' %(y))
        return 1

    return 0

def handleSTOU( s ):
    global homedir
    print('[TEST] handleSTOU')

    stoufd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    host = s.getsockname()[0]
    port = random.randint(4000, 20000)

    useport = (random.randint(0,100) > 50)

    if useport:
        print('\tUsing PORT')

        stoufd.bind((host, port))
        stoufd.listen(1)

        req = "PORT " + str(host).replace('.', ',') + ',' + str(int(port/256)) + ',' + str(port % 256) + "\n"

        send_wrapper(s, req)

        y = readuntil(s, '\r\n')

        if y.find("200 PORT command successful. Consider using PASV.") == -1:
            print('[ERROR] handleSTOU() port command failed: %s' %y)

            stoufd.close()
            return 1
    else:
        print('\tUsing PASV')
        send_wrapper(s, "PASV\r\n")

        y = readuntil(s, "\r\n")

        if y.find("227 Entering Passive Mode") == -1:
            print('[ERROR] handleSTOU() pasv command failed: %s' %y)
            return 1

        z = y.split(' ')[4]

        print(z)

        x = z.split(',')

        ip = x[0][1:] + '.' + x[1] + '.' + x[2] + '.' + x[3]
        port = int(x[4]) * 256 + int(x[5][:-4])

        print("IP: %s port: %d" %(ip, port))

        try:
            stoufd.connect((ip, port))
        except:
            print('[ERROR] handleSTOU() failed to connect to %s:%d' %(ip, port))
            return 1


    send_wrapper(s, 'TYPE I\n')

    y = readuntil(s, '\r\n')

    if y.find("200 Switching to BINARY mode.") == -1:
        print('[ERROR] handleSTOU() type command failed: %s' %y)
        return 1

    try:
        f = open('testfile', 'rb')
    except:
        print('[ERROR] handleSTOU() failed to open testfile. Should be in testfile')
        return 1

    filedata = f.read()
    f.close()

    send_wrapper(s, 'STOU\n')

    y = readuntil(s, '\r\n')

    if y.find("150 FILE:") == -1:
        print('[ERROR] handleSTOU() command failed: %s' %y)
        return 1

    nm = y.split(' ')[2].rstrip()

    if useport:
        c, addr = stoufd.accept()
        stoufd.close()
        stoufd = c

    b = ''
    r = ''
    try:
        print('[INFO] handleSTOU() sending data')
        stoufd.settimeout( .1 )
        stoufd.send(filedata)
    except:
        pass

    stoufd.close()

    print('[INFO] Sent the bytes')
    
    y = readuntil(s, '\r\n')

    if y.find('226 Transfer complete.') == -1:
        print('[ERROR] handleSTOU() failed: %s' %(y))
        return 1

    print('[INFO] Now deleting: %s' %(nm))

    send_wrapper(s, "DELE %s\n" %(nm))

    y = readuntil(s, '\r\n')

    if y.find('250 Delete operation successful.') == -1:
        print('[ERROR] DELE failed: %s' %(y))
        return 1

    return 0

def handleAPPE( s ):
    global homedir
    print('[TEST] handleAPPE')

    storfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    host = s.getsockname()[0]
    port = random.randint(4000, 20000)

    useport = (random.randint(0,100) > 50)

    if useport:
        print('\tUsing PORT')

        storfd.bind((host, port))
        storfd.listen(1)

        req = "PORT " + str(host).replace('.', ',') + ',' + str(int(port/256)) + ',' + str(port % 256) + "\n"

        send_wrapper(s, req)

        y = readuntil(s, '\r\n')

        if y.find("200 PORT command successful. Consider using PASV.") == -1:
            print('[ERROR] handleAPPE() port command failed: %s' %y)

            storfd.close()
            return 1
    else:
        print('\tUsing PASV')
        send_wrapper(s, "PASV\r\n")

        y = readuntil(s, "\r\n")

        if y.find("227 Entering Passive Mode") == -1:
            print('[ERROR] handleAPPE() pasv command failed: %s' %y)
            return 1

        z = y.split(' ')[4]

        print(z)

        x = z.split(',')

        ip = x[0][1:] + '.' + x[1] + '.' + x[2] + '.' + x[3]
        port = int(x[4]) * 256 + int(x[5][:-4])

        print("IP: %s port: %d" %(ip, port))

        try:
            storfd.connect((ip, port))
        except:
            print('[ERROR] handleSTOR() failed to connect to %s:%d' %(ip, port))
            return 1


    send_wrapper(s, 'TYPE I\n')

    y = readuntil(s, '\r\n')

    if y.find("200 Switching to BINARY mode.") == -1:
        print('[ERROR] handleAPPE() type command failed: %s' %y)
        return 1

    try:
        f = open('testfile', 'rb')
    except:
        print('[ERROR] handleAPPE() failed to open testfile. Should be in testfile')
        return 1

    filedata = f.read()
    f.close()

    send_wrapper(s, 'APPE %s/testfile\n' %(homedir))

    y = readuntil(s, '\r\n')

    if y.find("150 Ok to send data.") == -1:
        print('[ERROR] handleAPPE() command failed: %s' %y)
        return 1

    if useport:
        c, addr = storfd.accept()
        storfd.close()
        storfd = c

    b = ''
    r = ''
    try:
        print('[INFO] handleAPPE() sending data')
        storfd.settimeout( .1 )
        storfd.send(filedata)
    except:
        pass

    storfd.close()

    print('[INFO] Sent the bytes')
    
    y = readuntil(s, '\r\n')

    if y.find('226 Transfer complete.') == -1:
        print('[ERROR] handleAPPE() failed: %s' %(y))
        return 1

    return 0

def handleHELP( s ):
    print('[TEST] handleHELP')

    send_wrapper(s, "HELP\n");

    y = readuntil(s, '\r\n')

    if y.find("214-The following commands are recognized.") == -1:
        print('[ERROR] handleHELP() command failed: %s' %y)
        return 1

    y = readuntil(s, '\r\n')

    if y.find("ABOR ACCT ALLO APPE CDUP CWD  DELE EPRT EPSV FEAT HELP LIST MDTM MKD") == -1:
        print('[ERROR] handleHELP() command failed: %s' %y)
        return 1

    y = readuntil(s, '\r\n')

    if y.find("MODE NLST NOOP OPTS PASS PASV PORT PWD  QUIT REIN REST RETR RMD  RNFR") == -1:
        print('[ERROR] handleHELP() command failed: %s' %y)
        return 1

    y = readuntil(s, '\r\n')

    if y.find("RNTO SITE SIZE SMNT STAT STOR STOU STRU SYST TYPE USER XCUP XCWD XMKD") == -1:
        print('[ERROR] handleHELP() command failed: %s' %y)
        return 1

    y = readuntil(s, '\r\n')

    if y.find("XPWD XRMD") == -1:
        print('[ERROR] handleHELP() command failed: %s' %y)
        return 1

    y = readuntil(s, '\r\n')

    if y.find("214 Help OK.") == -1:
        print('[ERROR] handleHELP() command failed: %s' %y)
        return 1

    return 0

def handleMODE( s ):
    print('[TEST] handleMODE')

    send_wrapper(s, 'MODE S\n')

    y = readuntil(s, '\r\n')

    if y.find("200 Mode set to S.") == -1:
        print('[ERROR] handleHELP() command failed: %s' %y)
        return 1

    return 0

def handleREST(s):
    print('[TEST] handleREST')

    send_wrapper(s, 'REST 0\n')

    y = readuntil(s, '\r\n')

    if y.find('350 Restart position accepted') == -1:
        print('[ERROR] handleREST() command failed: %s' %y)
        return 1

    return 0

def handleREIN(s):
    print('[TEST] handleREIN')

    send_wrapper(s, 'REIN\n')

    y = readuntil(s, '\r\n')

    if y.find('220 Ok') == -1:
        print('[ERROR] handleREIN() command failed: %s' %y)
        return 1

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

    s = connect( HOST, PORT )

    if getbanner(s):
        print('[FAIL] Poller failed')
        sys.exit(1)
    else:
        print('[SUCCESS] Banner read')

    login(s)

    loggedin = 1
    for _ in range(15):
        if loggedin == 0:
            login(s)

        verb = random.choice([handleREIN, handleMODE, handleHELP, handleAPPE, handleSTOU, handleSTOR, handleRETR, handleRNFRRNTO, handleMDTM, handleXCWDCDUPMKDRMD, handleCWDCDUPMKDRMD, handleXMKDXRMD, handleMKDRMD, handleCWD, handleXCWD, handleLIST, handleNLST, handleNI, handleSYST, handleNOOP, handlePWD, handleXPWD, handleSTRU])
        if verb(s):
            print('[FAIL] Poller failed')
            sys.exit()

        if verb == handleREIN:
            loggedin = 0

    send_wrapper(s, "QUIT\n")
    
    print('[SUCCESS] Poller completed successfully')

    s.close()

if __name__ == '__main__':
    main()
