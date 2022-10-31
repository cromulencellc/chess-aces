#!/usr/bin/python3

import subprocess
import sys

def get_ip( interface: bytes ) -> bytes:
    output = subprocess.check_output([b'ifconfig', interface]).split(b'\n')

    ## strip any tabs
    output = list( map( lambda s: s.lstrip().rstrip(), output))

    line = [i for i in output if i.startswith(b'inet ')]

    assert len(line) == 1

    subfields = line[0].split(b' ')

    return subfields[1]

def insert_ip(file: bytes, ip: bytes) -> bytes:
    try:
        f = open(file, 'rb')
    except Eception as e:
        print(f'[ERROR] Failed to open {file} -- {e}')
        exit(1)

    data = f.read().split(b'\n')

    output_file = b''

    for line in data:
        if line.startswith(b'ipv4'):
            output_file += b'ipv4_bind_addresses="' + ip + b'"\n'
        else:
            output_file += line + b'\n'

    return output_file

if __name__ == '__main__':
    if len(sys.argv) == 1:
        iface = b'eth0'
    else:
        iface = sys.argv[1]

    if len(sys.argv) == 3:
        file = sys.argv[2]
    else:
        file = b'/etc/mararc'

    ip = get_ip(iface)

    print(f'Found IP: {ip}')

    output = insert_ip(file, ip)

    print(f'New config:\n\n{output}')

    f = open(file, 'wb')
    f.write(output)
    f.close()
