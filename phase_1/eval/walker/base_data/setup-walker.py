import sys
import shutil
import random
import os
import string
import subprocess
import time

def randomstring(l):
    q = ''

    for _ in range(l):
        q += random.choice(string.ascii_lowercase)

    return q

def main():

    if len(sys.argv) != 2:
        print('[USAGE] %s <user>' %(sys.argv[0]))
        print('[EXAMPLE] %s chess' %(sys.argv[0]))
        sys.exit(1)

    USER = sys.argv[1]

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    print('[INFO] This is use to setup the Maildir directory for the Walker Challenge')
    print('[INFO] It must be run as root')

    maildir = '/home/%s/Maildir' %(USER)

    if os.path.exists( maildir ):
        try:
            shutil.rmtree(maildir)
        except Exception as e:
            print('[ERROR] Failed to remove the %s. Are you root?' %(maildir))
            print(e)
            return None

    os.makedirs(maildir)

    for s in ['cur', 'new', 'tmp']:
        os.makedirs(maildir + '/' + s)

    cmd = "cp ./*.ubuntu-bionic %s/new/" %(maildir)
    os.system(cmd)

    for c in ['.childone', '.childtwo', '.childthr']:
        os.makedirs(maildir + '/' + c)
        for s in ['cur', 'new', 'tmp']:
                os.makedirs(maildir + '/' + c + '/' + s)

        cmd = "cp ./*.ubuntu-bionic %s/%s/new/" %(maildir, c)
        os.system(cmd)

        for d in ['.mba', '.mbb', '.mbc']:
            os.makedirs(maildir + '/' + c + '/' + d)
            for s in ['cur', 'new', 'tmp']:
                os.makedirs(maildir + '/' + c + '/' + d + '/' + s)

            cmd = "cp ./*.ubuntu-bionic %s/%s/%s/new/" %(maildir, c, d)
            os.system(cmd)

    cmd = 'sudo chown -R %s:%s %s' %(USER, USER, maildir)

    os.system(cmd)

    for c in [maildir, maildir + '/.childone', maildir + '/.childtwo', maildir + '/.childthr']:
        data = subprocess.check_output(["ls", c + '/new/'])

        data = data.decode('utf-8').split()

        if ( len(data) != 10 ):
            print('[ERROR] Something failed in the mail delivery process.')
            #sys.exit(1)

    print('[SUCCESS] Setup completed successfully')

if __name__ == '__main__':
    main()
