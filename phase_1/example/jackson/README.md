# jackson

This challenge is an irc client capable of basic interactions with an irc server.

## Install the server

The IRC server I used for testing was the ircd-irc2 daemon from the Ubuntu apt repository. I used the irssi Ubuntu client for testing the client communication functionality.

## Running the challenge

To run jackson you need to export two environment variables, HOST, and PORT. HOST can be set to "localhost" and you should set PORT to 6667 if you are running the IRC server on the default port. Otherwise, set it to whatever port you have configured.

If you want a user/nick combination that is different from default run:

export IRCUSER=<desired_username>
export IRCNICK=<desired_nick>

## Running the poller

Assuming that the irc server is up and running:

Set the "JACKSON" environment variable to the location of the challenge binary

export PORT=6667
export HOST=localhost

python jackson-poller.py

The pollers take an average of 3 minutes to complete for each run.

## Running the PoV

Set the "JACKSON" environment variable to the location of the challenge binary

netcat should be installed in the image.

python jackson-launch.py will launch the client and set up the netcat listener.

It will then launch the jackson-pov.py script which connects to the irc server and sends the exploit to the client.

