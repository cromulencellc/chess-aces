import sys
import string
import random
import os

def genpass( l ):

    password = b''

    for x in range(l):
        password += random.choice(list(string.ascii_letters + string.digits)).encode('UTF-8')

    return password

def main(config):
    f = open(config, 'rb')

    data = f.read().split(b'\n')

    f.close()

    p = genpass(32)

    final = []

    for l in data:
        w = l.split(b'\t\t')

        if w[0] == b'rootpw':
            w[1] = p

        x = b'\t\t'.join(w)

        final.append(x)

    outdata = b'\n'.join(final)

    f = open(config, 'wb')
    f.write(outdata)
    f.close()


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print(f'[USAGE] {sys.argv[0]} <slapd.conf>')
        sys.exit(1)

    if 'SEED' in os.environ:
        SEED = int(os.environ['SEED'])
    else:
        SEED = random.randint(0, 2**64 - 1)

    random.seed(SEED)

    main(sys.argv[1])
