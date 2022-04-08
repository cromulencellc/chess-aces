# pitsmoor

Pitsmoor is an injection bug into the open source project ngircd.

It is an IRC server so instead of covering the entire protocol here I recommend reading the RFC:

https://datatracker.ietf.org/doc/html/rfc1459

or Wikipedia:

https://en.wikipedia.org/wiki/Internet_Relay_Chat

## Scope for Analysis

The IRC server is in scope. The data contained in ``/token`` as well as the irc configuration file is considered privileged and regular users should not have access to either.

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

Memory Corruption

CWE-787 Out-of-bounds write

If an attacker is an operator in a channel and runs the "KICK" command with a long reason string then a buffer overflow can occur that will result in the overwriting of data on the stack.

### PoV 2

Authentication Bypass

In the IRC protocol a user can have the OPER mode 'o' which gives them access to additional privileged commands. For this challenge I added an additional IRC command called TOKEN that will send the token file to a user if they are an IRC operator. If an attacker provides a long info string when they connect via the USER command there is an off-by-one bug that allows them to write the letter 'o' in the modes array which makes them OPER. Once they are operator then they can issue the TOKEN command to leak the secret data.