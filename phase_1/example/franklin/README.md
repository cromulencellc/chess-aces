# franklin

docker-compose run --rm --name base base

docker-compose run --rm --workdir /poller -e HOST=base -e PORT=1337 poller python franklin-poller.py


## Info

This challenge parses provided network packet files. It can take a file name using the -s and -p command options or it can accept it via a network socket by first sending the 4 byte size of the data followed by the data itself. The max size of a packet file is 1024 bytes.
