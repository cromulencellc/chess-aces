import socket
import struct
import sys
import random
import os
import binascii
import time
import select

class Req_Header:
    def __init__(
                 self, protocol, command,
                 error_status, flags1, flags2, 
                 PIDhigh, reserved, TID, 
                 PIDlow, UID, MID
                 ):
        self.protocol = protocol
        self.commmand = command
        self.error_status = error_status
        self.flags1 = flags1
        self.flags2 = flags2
        self.PIDhigh = PIDhigh
        self.reserved = reserved
        self.TID = TID
        self.PIDlow = PIDlow
        self.UID = UID
        self.MID = MID

    def print_req_header(self):
        print('=====================================================\t')
        print('\tREQUEST HEADER\t')

        print('\tProtcol: ' + str(self.protocol))
        print('\tCommand: ' + str(self.commmand))
        print('\tError Status: ' + str(self.error_status))
        print('\tFlags1: ' + str(self.flags1))
        print('\tFlags2: ' + str(self.flags2))
        print('\tPIDhigh: ' + str(self.PIDhigh))
        print('\tReserved: ' + str(self.reserved))
        print('\tTID: ' + str(self.TID))
        print('\tPIDlow: ' + str(self.PIDlow))
        print('\tUID: ' + str(self.UID))
        print('\tMID: ' + str(self.MID))
        print('=====================================================\t\n\n')


class Param_Block:
    def __init__(self, word_count, words):
        self.word_count = word_count
        self.words = words

    def print_param_block(self):
        print('=====================================================\t')
        print('\tPARAMETER BLOCK\t')

        print('\tWord Count:' + str(hex(self.word_count)))
        print('\tWords: ' + str(hex(self.words)))
        print('=====================================================\t\n\n')

class Data_Block:
    def __init__(self, byte_count, dbytes):
        self.byte_count = byte_count
        self.dbytes = dbytes

    def print_data_block(self):
        print('=====================================================\t')
        print('\tDATA BLOCK\t')

        print('\tByte Count: ' + str(hex(self.byte_count)))
        print('\tDbytes: ' + str(hex(self.dbytes)))
        print('=====================================================\t\n\n')

class SMB_Message:
    def __init__(self, Req_Header, Param_Block, Data_Block):
        self.Req_Header = Req_Header
        self.Param_Block = Param_Block
        self.Data_Block = Data_Block

    def print_SMB_Message(self):

        print('=====================================================\t')
        print('\tSMB_Message\n')
        print('\tREQUEST HEADER\t')
        print('\tProtcol: ' + str(hex(self.Req_Header.protocol)))
        print('\tCommand: ' + str(hex(self.Req_Header.commmand)))
        print('\tError Status: ' + str(hex(self.Req_Header.error_status)))
        print('\tFlags1: ' + str(hex(self.Req_Header.flags1)))
        print('\tFlags2: ' + str(hex(self.Req_Header.flags2)))
        print('\tPIDhigh: ' + str(hex(self.Req_Header.PIDhigh)))
        print('\tReserved: ' + str(hex(self.Req_Header.reserved)))
        print('\tTID: ' + str(hex(self.Req_Header.TID)))
        print('\tPIDlow: ' + str(hex(self.Req_Header.PIDlow)))
        print('\tUID: ' + str(hex(self.Req_Header.UID)))
        print('\tMID: ' + str(hex(self.Req_Header.MID)))

        print('\tWords: ' + str(hex(self.Param_Block.words)))
        
        print('\n\tDATA BLOCK\t')
        print('\tByte Count: ' + str(hex(self.Data_Block.byte_count)))
        print('\tDbytes: ' + str(hex(self.Data_Block.dbytes)))
        print('=====================================================\t\n\n')

smb_commands = ['smb_com_create_dir',
                'smb_com_delete_dir',
                'smb_com_open',
                'smb_com_create',
                'smb_com_close',
                'smb_com_delete',
                'smb_com_rename',
                'smb_com_query_information',
                'smb_com_set_information',
                'smb_com_read',
                'smb_com_write',
                'smb_com_create_new',
                'smb_com_negotiate',
                'smb_com_setup_andx',
                ]

#reading: 0, writing: 1, reading & writing: 2
access_modes = ['0000','0100','0200']

open_fids = []

dir_test_names = ['/room1/',
                  '/room2/', 
                  '/room3/', 
                  '/room4/', 
                  '/room5/', 
                  '/room6/', 
                  '/room7/', 
                  '/room8/', 
                  '/room9/', 
                 ]

filenames = ['file1\x00',
             'file2\x00',            
             'file3\x00',            
             'file4\x00',            
             'file5\x00',            
             'file6\x00',            
             'file7\x00',            
             'file2\x00',            
             'file2\x00',            
             'file2\x00',            
             'file2\x00',            
             'file2\x00',            
             'file2\x00',            
             'file8\x00',            
             'file9\x00',            
            ]

versions = ['ABCDEFGH',
            'HGFEDCBA',
           ]

fid_path = []

ft_success = 0


def smb_com_create_dir(s1, dir_name):

    pid = os.getpid()
    print(pid)

    #Protocol
    packet = 'ff'
    packet += '534d42'

    #Command
    packet += '00'

    #Flags1
    packet += '00'

    #Flags2
    packet += '0000'

    #PIDhigh
    #Max PID on server is 32768 = 0x8000 PIDhigh never be nonzero
    packet += '0000'

    #Reserved
    packet += '0000'

    #TreeID REQUIRED
    packet += '0000'

    #PIDlow
    pidLow = "{:04x}".format(pid)

    pidLowBot = pidLow[:len(pidLow) // 2]
    pidLowTop = pidLow[len(pidLow) // 2:]

    pidLow = pidLowTop + pidLowBot
    packet += pidLow

    #UID REQUIRED
    packet += '0000'

    #MID
    packet += '0000'

    #Word Count NEEDS TO BE 00
    packet += '00'

    #Byte Count must be greater than or equal to 0x0002
    #because it will always be 2 bytes for the buffer format type
    #and if the dir_name is null it will be null term string
    packet += '0200'

    print(packet + dir_name)
    final_packet = bytes.fromhex(packet) + bytes(dir_name, 'utf-8')

    print(str(final_packet))
    s1.send(final_packet)

    #Response
    header = list()
    for i in range(0,24):
        header.append(s1.recv(1).hex())
    
    req_header = Req_Header(header[0:4], header[4], header[8:4:-1],
                            header[9], header[11:9:-1], header[13:11:-1],
                            header[15:13:-1], header[17:15:-1], header[19:17:-1],
                            header[21:19:-1], header[23:21:-1])

    for i in range(0,1):
        word_count = s1.recv(1).hex()

    print('Word Count: ' + str(word_count))

    byte_count = list()
    for i in range(0,2):
        byte_count.append(s1.recv(1).hex())

    print('Byte Count: ' + str(byte_count[::-1]))
    
    error_code = str(header[8]) +  str(header[7]) + str(header[6]) + str(header[5])

    global ft_success
    
    if error_code == '00000000':
        print('RETURN MESSAGE: SUCCESS')
        ft_success += 1
        return 2

    elif error_code == 'c000003b':
        print('RETURN MESSAGE: mkdir failed')
        return 0

    elif error_code == 'c0000035':
        print('RETURN MESSAGE: The directory already exists')
        return 1
    else:
        print('UNHANDLED MESSAGE ' + error_code)
        return 3

def smb_com_delete_dir(s1, dir_name):


    pid = os.getpid()

    #Protocol
    packet = 'ff'
    packet += '534d42'

    #Command
    packet += '01'

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
    packet += '00'

    #Byte Count
    packet += '0200'

    print(packet + dir_name)
    final_packet = bytes.fromhex(packet) + bytes(dir_name, 'utf-8')

    s1.send(final_packet)

    #Response
    header = list()
    for i in range(0,24):
        header.append(s1.recv(1).hex())

    req_header = Req_Header(header[0:4], header[4],header[8:4:-1],
                            header[9], header[11:9:-1], header[13:11:-1],
                            header[15:13:-1], header[17:15:-1], header[19:17:-1],
                            header[21:19:-1], header[23:21:-1])

    for i in range(0,1):
        word_count = s1.recv(1).hex()

    print('Word Count: ' + str(word_count))

    byte_count = list()
    for i in range(0,2):
        byte_count.append(s1.recv(1).hex())

    print('Byte Count: ' + str(byte_count[::-1]))

    error_code = str(header[8]) +  str(header[7]) + str(header[6]) + str(header[5])

    global ft_success

    if error_code == '00000000':
        print('RETURN MESSAGE: SUCCESS')
        ft_success += 1
        return 0

    elif error_code == 'c000000f':
        print('RETURN MESSAGE: The directory does not exist')
        return 1

    elif error_code == 'c000003b':
        print('RETURN MESSAGE: No directory name given')
        return 3

    elif error_code == 'c000003e':
        print('RETURN MESSAGE: The directory failed to be removed')
        return 4

    else:
        print('UNHANDLED MESSAGE ' + error_code)

def smb_com_open(s1, i, path):

    pid = os.getpid()

    #Protocol
    packet = 'ff'
    packet += '534d42'

    #Command
    packet += '02'

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
    packet += '02'

    #Words Access Mode
    #0 = read, 1 = write, 2 = read and write
    choice = ''

    if i  == -1:
        choice = random.choice(access_modes) 
        print(choice)

    elif i == 0:
        choice = access_modes[0] 
        print('Open for reading')

    elif i == 1:
        choice = access_modes[1] 
        print('Open for writing')

    elif i == 2:
        choice = access_modes[2] 
        print('Open for reading and writing')

    elif i == 8:
        choice = '0800'

    packet += choice 

    #Search Attributes always 0000, we are looking for normal files
    packet += '0000'

    #Byte Count
    packet += '0200'

    print(packet + path)
    final_packet = bytes.fromhex(packet) + bytes(path, 'utf-8')

    s1.send(final_packet)

    header = list()
    for i in range(0,24):
        header.append(s1.recv(1).hex())

    req_header = Req_Header(header[0:4], header[4],header[8:4:-1],
                            header[9], header[11:9:-1], header[13:11:-1],
                            header[15:13:-1], header[17:15:-1], header[19:17:-1],
                            header[21:19:-1], header[23:21:-1])

    #Word Count
    for i in range(0,1):
        word_count = s1.recv(1).hex()

    print('Word Count: ' + str(word_count))

    #Words
    words = list()
    for i in range(0,8):
        words.append(s1.recv(1).hex())

    print(str(words))

    #Byte Count
    byte_count = list()
    for i in range(0,2):
        byte_count.append(s1.recv(1).hex())

    print('Byte Count: ' + str(byte_count[::-1]))

    error_code = str(header[8]) +  str(header[7]) + str(header[6]) + str(header[5])

    global ft_success
    
    if error_code == '00000000':
        print('RETURN MESSAGE: SUCCESS')
        open_fids.append([str(words[0] + words[1]), path])
        ft_success += 1
        return 1

    elif error_code == 'c000000f':
        print('RETURN MESSAGE: The file does not exist')
        return 2

    elif error_code == 'c000003b':
        print('RETURN MESSAGE: No path name given')
        return 3

    elif error_code == 'c000003e':
        print('RETURN MESSAGE: There was a problem opening for writing/reading/access mode')
        return 4

    else:
        print('UNHANDLED MESSAGE ' + error_code)
        return 1

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

    req_header = Req_Header(header[0:4], header[4],header[8:4:-1],
                            header[9], header[11:9:-1], header[13:11:-1],
                            header[15:13:-1], header[17:15:-1], header[19:17:-1],
                            header[21:19:-1], header[23:21:-1])

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
    
    global ft_success
    
    if error_code == '00000000':
        print('RETURN MESSAGE: SUCCESS')
        ft_success += 1
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

def smb_com_close(s1,f):

    pid = os.getpid()

    #Protocol
    packet = 'ff'
    packet += '534d42'

    #Command
    packet += '04'

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
    #FID
    if f == 1:
        choice = random.choice(open_fids)
        packet += str(choice[0])

    elif f == 99:
        choice = '9900'
        packet += choice

    #Byte Count
    packet += '0000'

    print(packet)
    final_packet = bytes.fromhex(packet)

    s1.send(final_packet)

    #Response
    header = list()
    for i in range(0,24):
        header.append(s1.recv(1).hex())

    req_header = Req_Header(header[0:4], header[4],header[8:4:-1],
                            header[9], header[11:9:-1], header[13:11:-1],
                            header[15:13:-1], header[17:15:-1], header[19:17:-1],
                            header[21:19:-1], header[23:21:-1])

    #Word Count
    for i in range(0,1):
        word_count = s1.recv(1).hex()

    print('Word Count: ' + str(word_count))

    #Byte Count
    byte_count = list()
    for i in range(0,2):
        byte_count.append(s1.recv(1).hex())

    print('Byte Count: ' + str(byte_count[::-1]))

    error_code = str(header[8]) +  str(header[7]) + str(header[6]) + str(header[5])
    
    global ft_success
    
    if error_code == '00000000':
        print('RETURN MESSAGE: SUCCESS')
        open_fids.remove(choice)
        ft_success += 1
        return 0

    elif error_code == '00060001':
        print('RETURN MESSAGE: Failed to close the file, bad fid')
        return 1

    else:
        print('UNHANDLED MESSAGE ' + error_code)
        return 2


def smb_com_delete(s1,filename):

    pid = os.getpid()

    #Protocol
    packet = 'ff'
    packet += '534d42'

    #Command
    packet += '05'

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

    #SearchAttributes
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


    req_header = Req_Header(header[0:4], header[4],header[8:4:-1],
                            header[9], header[11:9:-1], header[13:11:-1],
                            header[15:13:-1], header[17:15:-1], header[19:17:-1],
                            header[21:19:-1], header[23:21:-1])

    #Word Count
    for i in range(0,1):
        word_count = s1.recv(1).hex()

    print('Word Count: ' + str(word_count))

    #Byte Count
    byte_count = list()
    for i in range(0,2):
        byte_count.append(s1.recv(1).hex())

    print('Byte Count: ' + str(byte_count[::-1]))

    error_code = str(header[8]) +  str(header[7]) + str(header[6]) + str(header[5])
    
    global ft_success
    
    if error_code == '00000000':
        print('RETURN MESSAGE: SUCCESS')
        ft_success += 1
        return 0

    elif error_code == 'c000000f':
        print('RETURN MESSAGE: There was an error deleting the file')
        return 1

    elif error_code == 'c000003b':
        print('RETURN MESSAGE: Directory name provided was NULL')
        return 2

    else:
        print('UNHANDLED MESSAGE ' + error_code)
        return 3

def smb_com_rename(s1, old, new):
    pid = os.getpid()

    #Protocol
    packet = 'ff'
    packet += '534d42'

    #Command
    packet += '06'

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
    #SearchAttr
    packet += '0000'

    #Byte Count
    packet += '0200'

    print(packet + old + new)
    final_packet = bytes.fromhex(packet) + bytes(old, 'utf-8') + bytes(new, 'utf-8')

    s1.send(final_packet)

    #Response
    header = list()
    for i in range(0,24):
        header.append(s1.recv(1).hex())

    req_header = Req_Header(header[0:4], header[4],header[8:4:-1],
                            header[9], header[11:9:-1], header[13:11:-1],
                            header[15:13:-1], header[17:15:-1], header[19:17:-1],
                            header[21:19:-1], header[23:21:-1])

    #Word Count
    for i in range(0,1):
        word_count = s1.recv(1).hex()

    print('Word Count: ' + str(word_count))

    #Byte Count
    byte_count = list()
    for i in range(0,2):
        byte_count.append(s1.recv(1).hex())

    print('Byte Count: ' + str(byte_count[::-1]))

    error_code = str(header[8]) +  str(header[7]) + str(header[6]) + str(header[5])
    
    global ft_success
    
    if error_code == '00000000':
        print('RETURN MESSAGE: SUCCESS')
        ft_success += 1
        return 0

    elif error_code == 'c000000f':
        print('RETURN MESSAGE: There was an error while renaming the file')
        return 1

    else:
        print('UNHANDLED MESSAGE ' + error_code)
        return 2


def smb_com_query_information(s1, filename):

    pid = os.getpid()

    #Protocol
    packet = 'ff'
    packet += '534d42'

    #Command
    packet += '07'

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
    packet += '00'

    #Byte Count
    packet += '0200'

    #Bytes
    #BufferFormat FileName 

    print(packet + filename)
    final_packet = bytes.fromhex(packet) + bytes(filename, 'utf-8')

    s1.send(final_packet)

    #Response
    header = list()
    for i in range(0,24):
        header.append(s1.recv(1).hex())


    req_header = Req_Header(header[0:4], header[4],header[8:4:-1],
                            header[9], header[11:9:-1], header[13:11:-1],
                            header[15:13:-1], header[17:15:-1], header[19:17:-1],
                            header[21:19:-1], header[23:21:-1])

    #Word Count
    for i in range(0,1):
        word_count = s1.recv(1).hex()

    print('Word Count: ' + str(word_count))

    words = list()
    for i in range(0,16):
        words.append(s1.recv(1).hex())

    print(str(words))

    #Byte Count
    byte_count = list()
    for i in range(0,2):
        byte_count.append(s1.recv(1).hex())

    print('Byte Count: ' + str(byte_count[::-1]))

    error_code = str(header[8]) +  str(header[7]) + str(header[6]) + str(header[5])
    
    global ft_success
    
    if error_code == '00000000':
        print('RETURN MESSAGE: SUCCESS')
        ft_success += 1
        return 0

    elif error_code == 'c000000f':
        print('RETURN MESSAGE: Stat failed')
        return 1

    else:
        print('UNHANDLED MESSAGE ' + error_code)
        return 2

def smb_com_set_information(s1, filename):

    pid = os.getpid()

    #Protocol
    packet = 'ff'
    packet += '534d42'

    #Command
    packet += '08'

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
    packet += '08'

    #Words
    #FileAttr
    packet += '0000'

    #LastWriteTime
    packet += '00000000'

    #Reserved
    packet += '00000000000000000000'

    #Byte Count
    packet += '0200'

    #Bytes
    #BufferFormat FileName 

    print(packet + filename)
    final_packet = bytes.fromhex(packet) + bytes(filename, 'utf-8')

    s1.send(final_packet)

    #Response
    header = list()
    for i in range(0,24):
        header.append(s1.recv(1).hex())

    req_header = Req_Header(header[0:4], header[4],header[8:4:-1],
                            header[9], header[11:9:-1], header[13:11:-1],
                            header[15:13:-1], header[17:15:-1], header[19:17:-1],
                            header[21:19:-1], header[23:21:-1])

    #Word Count
    for i in range(0,1):
        word_count = s1.recv(1).hex()

    print('Word Count: ' + str(word_count))

    #Byte Count
    byte_count = list()
    for i in range(0,2):
        byte_count.append(s1.recv(1).hex())

    print('Byte Count: ' + str(byte_count[::-1]))

    error_code = str(header[8]) +  str(header[7]) + str(header[6]) + str(header[5])
    
    global ft_success
    
    if error_code == 'c0000002':
        print('RETURN MESSAGE: NOT IMPLEMENTED')
        return 0

    else:
        print('UNHANDLED MESSAGE ' + error_code)
        return 1

def smb_com_read(s1, f):

    pid = os.getpid()

    #Protocol
    packet = 'ff'
    packet += '534d42'

    #Command
    packet += '09'

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
    packet += '05'

    #Words
    #FID
    if f == 1:
        fid = str(random.choice(open_fids)[0])
        print(fid)
        packet += fid 
    
    else:
        fid = '9900'
        print(fid)
        packet += fid

    #CountOfBytesToRead
    packet += '0000' 

    #EstimateOfRemainingBytesToBeRead
    packet += '0000'

    #Byte Count
    packet += '0000'

    print(packet)
    final_packet = bytes.fromhex(packet)

    s1.send(final_packet)

    #Response
    header = list()
    for i in range(0,24):
        header.append(s1.recv(1).hex())

    req_header = Req_Header(header[0:4], header[4],header[8:4:-1],
                            header[9], header[11:9:-1], header[13:11:-1],
                            header[15:13:-1], header[17:15:-1], header[19:17:-1],
                            header[21:19:-1], header[23:21:-1])

    #Word Count
    for i in range(0,1):
        word_count = s1.recv(1).hex()

    print('Word Count: ' + str(word_count))

    words = list()
    for i in range(0,10):
        words.append(s1.recv(1).hex())

    print(str(words))

    #Byte Count
    byte_count = list()
    for i in range(0,2):
        byte_count.append(s1.recv(1).hex())

    print('Byte Count: ' + str(byte_count[::-1]))

    dbytes = list()
    for i in range(0,int(byte_count[1] + byte_count[0], 16)):
        dbytes.append(s1.recv(1).hex())

    print(str(dbytes))

    error_code = str(header[8]) +  str(header[7]) + str(header[6]) + str(header[5])
    
    global ft_success
    
    if error_code == '00000000':
        print('RETURN MESSAGE: SUCCESS')
        ft_success += 1
        return 0

    elif error_code ==  '00060001':
        print('RETURN MESSAGE: There was a problem with read')
        return 1

    else:
        print('UNHANDLED MESSAGE ' + error_code)

def smb_com_write(s1, f, data):

    pid = os.getpid()

    #Protocol
    packet = 'ff'
    packet += '534d42'

    #Command
    packet += '0a'

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
    packet += '05'

    #Words
    #FID
    if f == 1:
        fid = str(random.choice(open_fids)[0])
        print(fid)
        packet += fid 
    
    else:
        fid = '9900'
        print(fid)
        packet += fid

    #BytesToWrite
    packet += str(binascii.hexlify((struct.pack('<Q',len(data)))).decode('utf-8'))[:4] 
    
    #Estimate
    packet += '0000'

    #Byte Count
    packet += '0300'

    #Bytes
    #Buffer Format
    packet += '01'

    #Data Length
    packet += '0500'

    print(packet + data)
    final_packet = bytes.fromhex(packet) + bytes(data, 'utf-8')

    s1.send(final_packet)

    #Response
    header = list()
    for i in range(0,24):
        header.append(s1.recv(1).hex())

    req_header = Req_Header(header[0:4], header[4],header[8:4:-1],
                            header[9], header[11:9:-1], header[13:11:-1],
                            header[15:13:-1], header[17:15:-1], header[19:17:-1],
                            header[21:19:-1], header[23:21:-1])

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
    
    global ft_success
    
    if error_code == '00000000':
        print('RETURN MESSAGE: SUCCESS')
        ft_success += 1
        return 1

    elif error_code ==  '00060001':
        print('RETURN MESSAGE: There was problem with write')
        return 2

    else:
        print('UNHANDLED MESSAGE ' + error_code)
        return 3

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

    req_header = Req_Header(header[0:4], header[4],header[8:4:-1],
                            header[9], header[11:9:-1], header[13:11:-1],
                            header[15:13:-1], header[17:15:-1], header[19:17:-1],
                            header[21:19:-1], header[23:21:-1])

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
    
    global ft_success
    
    if error_code == '00000000':
        print('RETURN MESSAGE: SUCCESS')
        ft_success += 1
        return 0

    elif error_code == 'c000003b':
        print('RETURN MESSAGE: Failed to create the file')
        return 1

    else:
        print('UNHANDLED MESSAGE ' + error_code)


def smb_com_negotiate(s1):
    pid = os.getpid()

    #Protocol
    packet = 'ff'
    packet += '534d42'

    #Command
    packet += '0c'

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
    packet += '00'

    #Byte Count
    packet += '1000'

    #Bytes
    #Buffer Format
    packet += '02'  

    #Dialect String
    dialect_str = 'NT LM 0.12'

    print(packet + dialect_str)
    final_packet = bytes.fromhex(packet) + bytes(dialect_str, 'utf-8')

    s1.send(final_packet)

    header = list()
    for i in range(0,24):
        header.append(s1.recv(1).hex())

    req_header = Req_Header(header[0:4], header[4],header[8:4:-1],
                            header[9], header[11:9:-1], header[13:11:-1],
                            header[15:13:-1], header[17:15:-1], header[19:17:-1],
                            header[21:19:-1], header[23:21:-1])
    
    word_count = s1.recv(1).hex()

    words = list()
    for i in range(0,34):
        words.append(s1.recv(1).hex())

    #dialect index 2 bytes
    dialect_index = words[0:2]
    print('Dialect Index: ' + str(dialect_index))

    #security mode 1 byte
    sec_mode = words[2:3]
    print('Security Mode: ' + str(sec_mode))

    #MaxMpxCount 2 bytes
    max_mpx_count = words[3:5]
    print('MaxMpxCount: ' + str(max_mpx_count))

    #MaxNumberVcs 2 bytes
    max_number_vcs = words[5:7]
    print('MaxMpxCount: ' + str(max_number_vcs))

    #MaxBufferSize 4 bytes
    max_buffer_size = words[7:11]
    print('Max Buffer Size: ' + str(max_buffer_size))

    #MaxRawSize 4 bytes
    max_raw_size = words[11:15]
    print('Max Raw Size: ' + str(max_raw_size))

    #Session Key 4 bytes
    session_key = words[15:19]
    print('Session Key: ' + str(session_key))

    #Capabilities 4 bytes
    capabilities =  words[19:23]
    print('Capabilities: ' + str(capabilities))

    #SystemTime 8 bytes
    system_time = words[23:31]
    print('System Time: ' + str(system_time))

    #SystemTimeZone 2 bytes
    system_time_zone = words[31:33]
    print('System Time Zone: ' + str(system_time_zone))

    #ChallengeLength 1 byte
    challenge_length = words[33:34]
    print('Challenge Length: ' + str(challenge_length))

    #Data Block
    #Byte Count
    byte_count = list()
    for i in range(0,2):
        byte_count.append(s1.recv(1).hex())

    print('Byte Count: ' + str(byte_count[::-1]))

    #Bytes
    dbyte = str(s1.recv(16).decode())

    print('Bytes: ' + dbyte)

def smb_com_setup_andx(s1):

    pid = os.getpid()

    #Protocol
    packet = 'ff'

    packet += '534d42'

    #Command
    packet += '0d'

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
    packet += '0D'

    #Words
    #AndXCommand
    packet += 'FF'

    #AndXReserved
    packet += '00'
    
    #AndXOffset
    packet += '0000'
    
    #AndXMaxBufferSize
    packet += '1104'
    
    #MaxMpxCount
    packet += '0001'
    
    #VcNumber
    packet += '0000'
    
    #SessionKey
    packet += '00000000'
    
    #OEMPasswordLen
    packet += '00'
    
    #UnicodePasswordLen
    packet += '00'
    
    #Reserved
    packet += '0000'
    
    #Capabilities
    packet += '0000000000000000'

    #Byte Count
    packet += '1000'

    #Bytes
    packet += '51'

    #OEMPassword
    packet += '54455354'

    #UnicodePassword
    packet += '00'

    #Pad
    packet += '00'

    #AccountName
    account_name = 'ADMIN'

    #PrimaryDomain
    prim_domain = 'FENWAY_WORKGROUP'

    #NativeOS
    nativeOS = 'Ubuntu 18.04'

    #Dialect String
    dialect_str = 'NT LM 0.12'

    print(packet + account_name + prim_domain + nativeOS + dialect_str)
    final_packet = bytes.fromhex(packet) + bytes(dialect_str, 'utf-8')

    s1.send(final_packet)

    header = list()
    for i in range(0,24):
        header.append(s1.recv(1).hex())

    req_header = Req_Header(header[0:4], header[4],header[8:4:-1],
                            header[9], header[11:9:-1], header[13:11:-1],
                            header[15:13:-1], header[17:15:-1], header[19:17:-1],
                            header[21:19:-1], header[23:21:-1])

    for i in range(0,1):
        word_count = s1.recv(1).hex()

    print('Word Count: ' + str(word_count))

    words = list()
    for i in range(0,6):
        words.append(s1.recv(1).hex())

    print('Words: ' + str(words))

    byte_count = list()
    for i in range(0,2):
        byte_count.append(s1.recv(1).hex())

    print('Byte Count: ' + str(byte_count[::-1]))

    #Bytes
    dbyte = list()
    for i in range(0,38):
        dbyte.append(s1.recv(1).hex())

    print('Bytes: ' + str(dbyte))


def function_test(HOST, PORT):

    buffer_format = '04'

    for i in range(0,16):    
    
        #Connect to the share negotiate and setup
        if i == 0:
            print('\n\nNEGOTIATE')

            s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
            s2.connect((HOST,PORT))

            smb_com_negotiate(s2)

            print('PASS[X] FAIL[]\n\n')

            s2.close()

        elif i == 1:
            print('\nSETUP_ANDX')

            s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
            s2.connect((HOST,PORT))

            smb_com_setup_andx(s2)

            print('PASS[X] FAIL[]\n\n')

            s2.close()

        #Create a directory create_dir
        elif i == 2:
            print('\nCREATE DIR')
            for j in range(0,2):
                s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
                s2.connect((HOST,PORT))
                
                if j == 0:
                    ret_val = smb_com_create_dir(s2, buffer_format + dir_test_names[0])

                    if ret_val == 2:
                        print('Directory Already Exists Check:  PASS[X] FAIL[]\n\n')

                    else:
                        print('Directory Already Exists Check:  PASS[] FAIL[X]')
                        sys.exit()

                if j == 1:
                    ret_val = smb_com_create_dir(s2, buffer_format + dir_test_names[0])

                    if ret_val == 1:
                        print('Success:  PASS[X] FAIL[]\n\n')

                    else:
                        print('Success:  PASS[] FAIL[X]')
                        sys.exit()

                
            s2.close()

        #Create a file inside that directory create
        elif i == 3:
            print('\nCREATE')

            s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
            s2.connect((HOST,PORT))

            ret_val = smb_com_create(s2, buffer_format + versions[0] + dir_test_names[0] + filenames[0])

            if ret_val == 1:
                print('SUCCESS:  PASS[X] FAIL[]\n\n')
            
            else:
                print('SUCCESS:  PASS[] FAIL[X]')
                sys.exit()

            s2.close()
        
        #Open that file for writing open      
        elif i == 4:
            print('\nOPEN for Write')
            
            for j in range(0,3):

                s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
                s2.connect((HOST,PORT))

                if j == 0:
                    ret_val = smb_com_open(s2, 1, buffer_format + dir_test_names[0] + filenames[0])

                    if ret_val == 1:
                        print('SUCCESS:  PASS[X] FAIL[]\n\n')
                    else:
                        print('SUCCESS:  PASS[] FAIL[X]')
                        sys.exit()

                if j == 1:
                    ret_val = smb_com_open(s2, 8, buffer_format + dir_test_names[0] + filenames[0])

                    if ret_val == 4:
                        print('Something was wrong with the access mode:  PASS[X] FAIL[]\n\n')
                    else:
                        print('Something was wrong with the access mode:  PASS[] FAIL[X]')
                        sys.exit()
                
                if j == 2:
                    ret_val = smb_com_open(s2, 1, buffer_format + dir_test_names[0] + filenames[1])

                    print(str(ret_val))

                    if ret_val == 2:
                        print('The file does not exist:  PASS[X] FAIL[]\n\n')
                    else:
                        print('The file does not exist:  PASS[] FAIL[X]')
                        sys.exit()

                s2.close()

        #Write to that File write
        elif i == 5:
            print('\nWRITE')

            for j in range(0,2):

                s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
                s2.connect((HOST,PORT))

                if j == 0:
                    ret_val = smb_com_write(s2, 1, 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA')

                    if ret_val == 1:
                        print('SUCCESS:  PASS[X] FAIL[]\n\n')
                    else:
                        print('SUCCESS:  PASS[] FAIL[X]')
                        sys.exit()

                if j == 1:
                    ret_val = smb_com_write(s2, 99, 'BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB')

                    if ret_val == 2:
                        print('Invalid Fid:  PASS[X] FAIL[]\n\n')
                    else:
                        print('Invalid Fid:  PASS[] FAIL[X]')
                        sys.exit()
                
                s2.close()

        #Close that File close
        elif i == 6:
            print('\nCLOSE')

            for j in range(0,2):

                s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
                s2.connect((HOST,PORT))

                if j == 0:
                    ret_val = smb_com_close(s2, 1)

                    if ret_val == 0:
                        print('SUCCESS:  PASS[X] FAIL[]\n\n')
                    else:
                        print('SUCCESS:  PASS[] FAIL[X]')
                        sys.exit()

                if j == 1:
                    ret_val = smb_com_close(s2, 99)

                    if ret_val == 1:
                        print('Invalid Fid:  PASS[X] FAIL[]\n\n')
                    else:
                        print('Invalid Fid:  PASS[] FAIL[X]')
                        sys.exit()
                
                s2.close()

        #Open that file for reading open
        elif i == 7:
            print('\nOPEN for Read')

            for j in range(0,2):

                s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
                s2.connect((HOST,PORT))

                if j == 0:
                    ret_val = smb_com_open(s2, 0, buffer_format + dir_test_names[0] + filenames[0])

                    if ret_val == 1:
                        print('SUCCESS:  PASS[X] FAIL[]\n\n')
                    else:
                        print('SUCCESS:  PASS[] FAIL[X]')
                        sys.exit()

                if j == 1:
                    ret_val = smb_com_open(s2, 8, buffer_format + dir_test_names[0] + filenames[0])

                    if ret_val == 4:
                        print('Something was wrong with the access mode:  PASS[X] FAIL[]\n\n')
                    else:
                        print('Something was wrong with the access mode:  PASS[] FAIL[X]')
                        sys.exit()
                
                s2.close()


            s2.close()

        #Read that file read
        elif i == 8:
            print('\nREAD')

            for j in range(0,2):

                s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
                s2.connect((HOST,PORT))

                if j == 0:
                    ret_val = smb_com_read(s2, 1)

                    if ret_val == 0:
                        print('SUCCESS:  PASS[X] FAIL[]\n\n')
                    else:
                        print('SUCCESS:  PASS[] FAIL[X]')
                        sys.exit()

                if j == 1:
                    ret_val = smb_com_read(s2, 99)

                    if ret_val == 1:
                        print('Invalid Fid:  PASS[X] FAIL[]\n\n')
                    else:
                        print('Invalid Fid:  PASS[] FAIL[X]')
                        sys.exit()
                
                s2.close()

        #Query information about the file QI
        elif i == 9:
            print('\nQUERY INFORMATION')

            s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
            s2.connect((HOST,PORT))

            ret_val = smb_com_query_information(s2, buffer_format + dir_test_names[0] + filenames[0])

            if ret_val == 0:
                print('SUCCESS:  PASS[X] FAIL[]\n\n')
            else:
                print('SUCCESS:  PASS[] FAIL[X]')
                sys.exit()

            s2.close()

        #Set information aobut the file SI
        elif i == 10:
            print('\nSET INFORMATION')

            s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
            s2.connect((HOST,PORT))

            smb_com_set_information(s2, buffer_format + dir_test_names[0] + filenames[0])

            if ret_val == 0:
                print('NOT IMPLEMENTED:  PASS[X] FAIL[]\n\n')
            else:
                print('NOT IMPLEMENTED:  PASS[] FAIL[X]')
                sys.exit()

            s2.close()

        #Delete the file delete the delete
        elif i == 11:
            print('\nDELETE')

            s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
            s2.connect((HOST,PORT))

            ret_val = smb_com_delete(s2, buffer_format + dir_test_names[0] + filenames[0])

            if ret_val == 0:
                print('SUCCESS:  PASS[X] FAIL[]\n\n')
            else:
                print('SUCCESS:  PASS[] FAIL[X]')
                sys.exit()
            
            s2.close()

        #Create the file again create_new
        elif i == 12:
            print('\nCREATE_NEW')

            s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
            s2.connect((HOST,PORT))

            ret_val = smb_com_create_new(s2, buffer_format + versions[1] + dir_test_names[0] + filenames[0])

            if ret_val == 0:
                print('SUCCESS:  PASS[X] FAIL[]\n\n')
            else:
                print('SUCCESS:  PASS[] FAIL[X]')
                sys.exit()

            s2.close()

        #Rename the file rename
        elif i == 13:
            print('\nRENAME')

            s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
            s2.connect((HOST,PORT))


            for j in range(0,2):

                s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
                s2.connect((HOST,PORT))

                if j == 0:
                    ret_val = smb_com_rename(s2, buffer_format + dir_test_names[0] + filenames[0], buffer_format + dir_test_names[0] + filenames[1])

                    if ret_val == 0:
                        print('SUCCESS:  PASS[X] FAIL[]\n\n')
                    else:
                        print('SUCCESS:  PASS[] FAIL[X]')
                        sys.exit()

                if j == 1:
                    ret_val = smb_com_rename(s2, buffer_format + dir_test_names[0] + filenames[3], buffer_format + dir_test_names[0] + filenames[1])

                    if ret_val == 1:
                        print('Rename Failed:  PASS[X] FAIL[]\n\n')
                    else:
                        print('Rename Failed:  PASS[] FAIL[X]')
                        sys.exit()
                
                s2.close()

            s2.close()

        #Delete the file
        elif i == 14:
            print('\nDELETE')

            s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
            s2.connect((HOST,PORT))

            ret_val = smb_com_delete(s2, buffer_format + dir_test_names[0] + filenames[1])

            if ret_val == 0:
                print('SUCCESS:  PASS[X] FAIL[]\n\n')
            else:
                print('SUCCESS:  PASS[] FAIL[X]')
                sys.exit()
            
            s2.close()

        #Delete the directory contained in the file
        elif i == 15:
            print('\nDELETE_DIR\n\n')

            for j in range(0,2):
                s2 = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
                s2.connect((HOST,PORT))
                
                if j == 0:
                    ret_val = smb_com_delete_dir(s2, buffer_format + dir_test_names[0])

                    if ret_val == 0:
                        print('Success:  PASS[X] FAIL[]\n\n')

                    else:
                        print('Success:  PASS[] FAIL[X]')
                        sys.exit()


                if j == 1:
                    ret_val = smb_com_delete_dir(s2, buffer_format + dir_test_names[0])

                    if ret_val == 1:
                        print('Directory Doesn\'t Exists Check:  PASS[X] FAIL[]\n\n')

                    else:
                        print('Directory Doesn\'t Exists Check:  PASS[] FAIL[X]')
                        sys.exit()

            s2.close()

    print('======================FUNCTION TEST REPORT======================')

    if ft_success == 13:
        print('All functions recieved correct response')
        print('Function Test PASSED')

    else:
        print('Function test FAILED')
        print('FT VALUE: ' + str(ft_success))

    print('================================================================')


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

    function_test(HOST,PORT)

    print('End of execution')

if __name__ == '__main__':
    main()

