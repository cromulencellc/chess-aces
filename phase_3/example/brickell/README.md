# brickell
Brickell is based on the open source project lighttpd. https://www.lighttpd.net

It uses the HTTP protocol; mozilla developer docs: https://developer.mozilla.org/en-US/docs/Web/HTTP

## Build
```
1. ./configure
2. make && make install
```

## Run
```
/challenge/sbin/lighttpd -f /data/lighttpd.conf -D
```

## Running in Docker
To build & run challenge
```
docker-compose -f docker-compose.yaml build ta3_brickell
docker-compose up ta3_brickell
```
To build & run poller
```
docker-compose -f docker-compose.yaml build ta3_brickell_poller
docker-compose up ta3_brickell_poller
```
to build & run pov_x, where x is 1 or 2 (the number of the pov)
```
docker-compose -f docker-compose.yaml build ta3_brickell_pov_x
docker-compose up ta3_brickell_pov_x
```
## Generating a data directory for poller and challenge

You'll want ruby for this.

ruby priv/filer.rb base_data outputs a directory tree for /data into the base_data directory; you can pass in a different path for that second argument.

The poller and challenge both need this in /data

There's a cheatsheet file there that the poller uses but the challenge doesn't. It includes passwords that are otherwise stored in hashes that a normal user would not have access to. This file is considered privileged data
POV authors MUST NOT assume the existence or contents of this file, or knowledge of any of the passwords in it.

## Scope for Analysis

All code included in this challenge is in scope. The data contained in /token is considered privileged data and should not normally be accessible.

## POV 1
CWE-22/20/200: Improper Limitation of a Pathname to a Restricted Directory ('Path Traversal')/Improper Input Validation/Exposure of Sensitive Information to an Unauthorized Actor

The challenge fails to prevent the user from specifying a user directory as /~../. This leads to a path traversal to the root directory. The user is then able to obtain sensitive data in the form of the token file.

## POV 2
CWE-121 : Stack-based Buffer Overflow

The challenge allows the user to corrupt the stack buffer with user defined code, which may overflow the stack leading to RIP and register control - leading to arbitrary code execution.

The user must provide a properly formed 'other'-type of header key (specifcally GPC-SEC) and a malformed value larger than the buf which will cause the stack to be overwritten. Entering random data of a too large size will lead to crashing the server as well; normally it would just restart. Therefore, the intent is to provide a properly formed shellcode that allows for arbitrary code execution.
