# adams
adams is an smtp server with basic authentication capabilities

You need to create a chess user and set the password to chess.

Add the line "mesg y" to the .bashrc file of chess.

Login as chess and run the challenge with that user.

The /base_data folder should be copied to the root directory and be readable by the chess user.

docker-compose run --rm --name base base

docker-compose run --rm --workdir /poller -e HOST=base -e PORT=1337 poller python adams-poller.py

