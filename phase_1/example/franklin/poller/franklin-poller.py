import sys
import socket
import random
import time
import base64
import os
import datetime
import struct
import string

def connect( ip, port ):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.connect( (ip, port) )
    except:
        print 'Failed to connect to %s:%d' %(ip, port)
        sys.exit(1)

    return s

def rs(l):
    z = ''

    for _ in range(l):
        z += random.choice( string.lowercase )

    return z

def readuntil( s, u ):
    z = ''

    try:
        while z.endswith( u ) == False:
            z += s.recv(1)
    except Exception as e:
        print '[ERROR]'
        print (e)
        return None

    return z

def readline( s ):
    return readuntil( s, '\n')

def rd( l ):
    z = ''
    for _ in range(l):
        z += chr( random.randint(0,255))

    return z

def sb( i ):
    return struct.pack('B', i)

def rsb():
    return sb( random.randint(0,255))

def sh( i ):
    return struct.pack('H', i)

def rsh():
    return sh( random.randint(0, 65535))

def si(i):
    return struct.pack('I', i)

def rsi():
    return si( random.randint(0, 2**32 - 1))


def eigrp( ):
    v = rsb()
    opc = rsb()
    cs = rsh()
    fl = rsi()
    seq = rsi()
    ack = rsi()
    vrid = rsh()
    autosy = rsh()
    par = rsh()

    l = random.randint( 5, 50)

    d = ''

    for i in range(l-4):
        d += rsb()

    l = sh(socket.htons(l))

    sv = rsh()
    svl = rsh()
    rl = rsh()
    tlv = rsh()

    pkt = v + opc + cs + fl + seq + ack + vrid + autosy + par + l + d + sv + svl + rl + tlv

    return pkt

def igmp( ):
    tp = struct.pack('B', random.choice( [0x11, 0x12, 0x16, 0x17, 0x22]))

    mt = struct.pack('B', random.randint(0, 255))

    cs = struct.pack('H', random.randint(0, 65535))

    gi = ''

    for i in range(4):
        gi += chr(random.randint(0, 255))

    pk = tp + mt + cs + gi

    return pk

def addoptions( ):
        ### todo add icmpv6 options
    icmpv6_option = random.choice( [ 1,2,3,5,14,24,25,31])

    pkt = sb( icmpv6_option)
    if icmpv6_option in [1,2]:
        length = 0

        addr = rd(6)

        length = (len(addr) + 2) / 8
        pkt += sb( length ) + addr
    elif icmpv6_option == 3:
        length = 0

        pl = sb(16)

        flags = rsb()

        vl = rsi()

        prl = rsi()

        prefix = rd(16)

        icd = pl + flags + vl + prl + prefix

        if ( len(icd) + 2) %8:
            icd += '\x00'*(8- ((len(icd)+2)%8))

        length = sb( (len(icd) + 2) / 8)

        pkt += length + icd
    elif icmpv6_option in [5, 14]:
        icd = rsh() + rsi()

        if ( len(icd) + 2) %8:
            icd += '\x00'*(8- ((len(icd)+2)%8))

        pkt += sb( (len(icd) + 2) / 8) + icd
    elif icmpv6_option == 24:
        pl = sb(16)
        flags = rsi()
        rl = rsi()
        pre = rd(16)

        icd = pl + flags + rl + pre

        if ( len(icd) + 2) %8:
            icd += '\x00'*(8- ((len(icd)+2)%8))

        pkt += sb( (len(icd) + 2) / 8 )+ icd
    elif icmpv6_option == 25:
        cnt = random.randint( 5, 10)

        icd = rsh() + rsi()

        for i in range(cnt):
            icd += rd(16)

        pkt += sb( (len(icd) + 2) / 8) + icd
    elif icmpv6_option == 31:
        dns = random.randint( 5, 10 )

        icd = rsh() + rsi()

        for i in range(dns):
            host = rs( 4 )
            domain = rs( 8 )
            tld = random.choice(['net', 'com', 'org'])

            icd += sb( len(host)) + host
            icd += sb( len(domain)) + domain
            icd += sb( len(tld)) + tld
            icd += sb(0)

        icd += sb(0)*16

        if len(icd) % 64:
            icd += '\x00'*(64- ((len(icd)+2)%64))


        pkt += sb( (len(icd) + 2) / 8 ) + icd


    return pkt

def icmpv6():

    tp = random.choice( [1, 2, 3,4, 100, 101, 127, 128, 129, 130, 131, 132, 133, 133, 134, 135, 136, 137, 138,
                         139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 151, 152, 153, 155, 200, 201, 205])

    code = rsb()

    cs = rsh()
    
    pkt = sb( tp )

    if tp == 2:
        pkt += code + cs + rsi()
    elif tp == 128:
        pkt += code + cs + rsh() + rsh() + rd( random.randint( 5,10))
    elif tp == 129:
        pkt += code + cs + rsh() + rsh() + rd( random.randint( 5,10))
    elif tp == 134:
        hl = rsb()
        flags = rsb()
        rl = rsh()
        rt = rsi()
        rett = rsi()

        pkt += code + cs + hl + flags + rl + rt + rett

    elif tp == 135:
        pkt += code + cs + rsi() + rd( 16 )
    elif tp == 136:
        pkt += code + cs + rsi() + rd(16)
    elif tp == 139:
        pkt += sb(0) + cs + sh( random.randint(0, 4)) + rsh() + rsi() + rd(16)
    elif tp == 140:
        code = sb( random.randint(0,2))

        qt = random.randint(0,4)

        nonce = rd(8)

        pkt += code + cs + sh(qt) + rsh() + nonce

        if qt == 2:
            ttl = rsi()

            pkt += rsi() ## ttl

            l = random.randint(5,30)

            print 'L: %d' %(l)
            pkt += sb( l )

            pkt += rd(l+1)

            pkt += sb(0)


        pkt += rd( random.randint(0, 10) )
    elif tp == 143:
        pkt += code + cs + rsh()

        nr = random.randint(1, 3)

        print hex(socket.htons(nr))
        pkt += sh(socket.htons(nr))

        for i in range(nr):
            rt = rsb()
            adl = rsb()
            ns = rsh()
            ns = sh( socket.htons( random.randint(0, 65535) ) )
            addr = rd(16)

            pkt += rt + adl + ns + addr
    else:
        pkt += code + cs 


    pkt += addoptions()

    return pkt


def icmpv4( ):
    tp = struct.pack('B', random.choice( [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 30, 42, 43]))

    code = struct.pack('B', random.randint(0, 15 ))

    ### Todo implement icmp checksum
    checksum = struct.pack('H', random.randint(0, 65535))

    return tp + code + checksum

def udp():
    sport = struct.pack('H', random.randint(0,65535))

    dport = struct.pack('H', random.randint(0,65535))

    length = random.randint(5, 400) + 8

    cs = struct.pack('H', random.randint(0,65535))

    d = ''

    for x in range(length - 8):
        d += chr( random.randint(0, 255))

    length = struct.pack('>H', length )

    pkt = sport + dport + length + cs + d

    return pkt

def tcp( ):
    sport = struct.pack('H', random.randint(0,65535))

    dport = struct.pack('H', random.randint(0,65535))

    seqno = struct.pack('I', random.randint(0, (2**32 - 1) ))

    ackno = struct.pack('I', random.randint(0, (2**32 - 1) ))

    do_rf = struct.pack('H', 0)

    ws = struct.pack('H', random.randint(0,65535))

    cs = struct.pack('H', random.randint(0,65535))

    urgptr = struct.pack('H', random.randint(0,65535))

    pkt = sport + dport + seqno + ackno + do_rf + ws + cs + urgptr

    return pkt

def ipv6():
    b = struct.pack('B', 0x60)
    b += '\x00\x00\x00'

    length = 0 ## 2 bytes

    nh = random.choice( [ 58, 6, 88])  ## byte

    hl = struct.pack('B', random.randint(0, 255))

    src = ''

    for i in range( 16 ):
        src += chr( random.randint( 0, 255))

    dst = ''

    da = ''
    for i in range( 16 ):
        dst += chr( random.randint( 0, 255))

    if nh == 6:
        d = tcp()
    elif nh == 58:
        d = icmpv6()
    elif nh == 88:
        d = eigrp()

    length = len(d)

    pkt = b + struct.pack('>H', length) + struct.pack('B', nh) + hl + src + dst + d

    return pkt


def ipv4( ):
    vl = struct.pack('B', 0x45)

    dscp_en = struct.pack('B', random.randint(0, 255))

    ### I don't know this one yet
    length = 0

    idi = struct.pack('H', random.randint(0, 65535))

    flgs_off = struct.pack('H', 0)

    ttl = struct.pack('B', random.randint(0, 255))

    ### Todo make better
    protocol = random.choice([1, 2, 6, 17, 41])

    ### I don't actually check this
    checksum = struct.pack('H', random.randint(0, 65535))

    si = ''
    for i in range(4):
        si += chr( random.randint(0,255))

    di = ''

    for i in range(4):
        di += chr( random.randint(0,255))


    if protocol == 1:
        d = icmpv4( )
    elif protocol == 2:
        d = igmp()
    elif protocol == 6:
        d = tcp()
    elif protocol == 17:
        d = udp()
    elif protocol == 41:
        d = ipv6()

    length = struct.pack('>H', len(d))

    packet = vl + dscp_en + length + idi + flgs_off + ttl + struct.pack('B', protocol) + checksum + si + di + d

    return packet

def arp():
    htype = struct.pack('>H', random.choice( [1, 6, 7, 15, 16, 17, 18, 19, 20]))

    prot = struct.pack('H', 0x0008)   ## ipv4

    hln = struct.pack('B', 6)
    pln = struct.pack('B', 4)

    opcode = struct.pack('>H', random.randint(1,9))

    sm = ''
    for i in range(6):
        sm += chr(random.randint( 0, 255))

    si = ''
    for i in range(4):
        si += chr ( random.randint(0, 255))

    dm = ''

    for i in range(6):
        dm += chr(random.randint( 0, 255))

    di = ''
    for i in range(4):
        di += chr ( random.randint(0, 255))


    packet = htype + prot + hln + pln + opcode + sm + si + dm + di

    return packet


def ethloop( ):
    skipcount = struct.pack('H', random.randint(0, 65535))
    function = struct.pack('H', random.randint(0, 65535))
    receipt_no = struct.pack('H', random.randint(0, 65535))

    dl = random.randint( 1, 255)

    packet = skipcount + function + receipt_no

    for i in range(dl):
        packet += chr( random.randint(0, 255))

    return packet

def ethernet_header( s ):
    dest = ''

    packet = ''

    for i in range(6):
        dest += chr(random.randint( 0, 255))

    src = ''

    for i in range(6):
        src += chr(random.randint( 0, 255))

    t = random.choice([0x0008, 0x0608, 0x0090, 0xdd86])

    packet = dest + src + struct.pack('H', t)

    if t == 0x0090:
        print '[INFO] Testing Ethernet LOOP'
        ### LOOP
        packet += ethloop( )
        s.send(struct.pack('I', len(packet)) + packet)
        ### TODO set a time out. If I never receive done then fail
        y =  readuntil(s, 'Done\n')
    elif t == 0x0608:
        print '[INFO] Testing Ethernet ARP'
        packet += arp( )
        s.send(struct.pack('I', len(packet)) + packet)
        y =  readuntil(s, 'Done\n')
    elif t == 0x0008:
        print '[INFO] Testing IPv4'
        packet += ipv4( )
        s.send(struct.pack('I', len(packet)) + packet)
        y = readuntil(s, 'Done\n')
    elif t == 0xdd86:
        print '[INFO] Testing IPv6'
        packet += ipv6()
        s.send( struct.pack('I', len(packet)) + packet)
        y = readuntil(s, 'Done\n')
        
    print y

    if y == None:
        print 'returned none'
        return 0

    if y.find('ERROR') != -1 or y.find('Insuff') != -1:
        return 0

    return 1

        
def run_test(HOST, PORT):
    s = connect( HOST, PORT )

    r = ethernet_header(s)

    s.close()

    if r == 0:
        print '[FAILED] Test did not succeed'
        sys.exit()

def main():
    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
    else:
        if 'HOST' in os.environ and 'PORT' in os.environ:
            HOST = os.environ['HOST']
            PORT = int(os.environ['PORT'])
        else:
            print '[ERROR] target and port must be specified via arguments or variables.'
            sys.exit(1)

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print '[INFO] Poller for the Franklin Challenge Binary'
    print '[INFO] SEED: {0}'.format(SEED)

    random.seed( SEED )

    run_test(HOST, PORT)
    print '[SUCCESS] Poller completed successfully'

if __name__ == '__main__':
    main()