# Inagua

Inagua is an injection bug into the open source project ngircd.

It is an IRC server so instead of covering the entire protocol here I recommend reading the RFC:

https://datatracker.ietf.org/doc/html/rfc1459

or Wikipedia:

https://en.wikipedia.org/wiki/Internet_Relay_Chat


The IRC server is in scope. The data contained in ``/token`` as well as the irc configuration file is considered privileged and regular users should not have access to either.

## Overview
Externality: ngircd 26.1

Protocol: IRC

Language: C

## Running 

ngircd needs to be run with the following options:

``-n`` disable forking.
``-f`` Specify the configuration file.

For this challenge use the configuration file "sample-ngircd.conf" under the "docs" directory

```
/home/challenge/ngircd/src/ngircd/ngircd -n -f ./doc/sample-ngircd.conf

```

## Connecting

The easiest way to connect and communicate with the server is via an irc client such as ``irssi``. For usage information for whichever client that is used check the man page.

## POVs

### PoV 1

Data Misuse
Resource Consumption

CWE-416: Use After Free
CWE-834: Excessive Iteration

In IRC users are able to join channels and if they have permission they are able to modify the topic of a channel. When a user exits a room, if they are the last person there then the channel meta structures are freed since they are no longer needed. I inserted two pieces of code. First was a new command 'TOPDEL' that allows a user that has permission to delete the topic of a given channel. This causes a NULL pointer to be written to the channel structure. If user exits the channel when the server attempts to free all the buffers in use and see that the topic pointer is NULL. This will result in an error condition where the channel gets freed but it is not removed from the linked list of available channels. If a user then joins two more channels this will result in using the previously freed structure which will cause the linked list to become circular and loop forever. The server will become unresponsive and CPU usage will spike to 100% (assuming only one core).

The reference PoV results in a 100% CPU spike of the ngircd process. To view this use ``docker exec`` to run ``top`` in bash.

### PoV 2

Information Disclosure

CWE-170: Improper Null Termination

Using the SUMMON command, if a user sends a message 72 bytes or longer it will result in a failure to terminate with a null byte. This allows an attacker to leak a heap address.
