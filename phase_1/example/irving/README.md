## irving

irving acts like a shell when connected to with the ability to manipulate the underlying file system.

It expects that a chess user exists and needs to be run as root so that it is able to drop privileges.

## Poller

The poller expects two files in the chess home directory

Here is the python dictionary definition:

files = {'/home/chess/data': 'helloworld\n', '/home/chess/data2': 'chess\nstuff\n'}

## Poller

Each iteration of the poller trashes the /home/chess directory so it must be reset.