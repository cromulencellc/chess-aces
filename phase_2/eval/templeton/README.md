# templeton

This challenge is a typical webserver implementing the following methods:

GET
HEAD
POST
PUT
DELETE
CONNECT

Since this challenges follows the RFC specifications one of the best resources that I found were the mozilla development docs.

https://developer.mozilla.org/en-US/docs/Web/HTTP

In addition there is a custom heap allocator used instead of libc's malloc().

The custom allocator is a typical setup for dynamic memory allocation.

## Scope for Analysis

The compiled binary ``templeton.bin`` is in scope. The data in ``/token`` is privileged data and should not be accessible to a normal user. A PoV can depend on the data contained in the files under ``/data``.

The files under ``/data`` should not be able to be modified.

## Files in the webroot

### file-upload-manager.php

Back end of the ``file-upload.html`` pages that handles the submission of a file.

### file-upload.html

Used to submit a file to the chess web server

### index.html

Basic landing page for the web server.

### welcome.php

Test php script with some basic input abilities for a name and an email address.

## Running

To run this as a server you need to set the ``PORT`` environment variable to the port on which you want to listen as well as the CHESS testbed environment variable.

```
CHESS=chess PORT=8888 ./build/templeton.bin
[INFO] Listener socket on port: 8888
```

If you want the challenge to accept input from stdin then you can add the ``-s`` flag.

```
CHESS=chess ./build/templeton.bin -s
```

## Example server connection with netcat

```
echo -en 'GET /index.html HTTP/1.1\r\nHost: chess\r\n\r\n' | nc localhost 8888
HTTP/1.1 200 OK
Content-Type: text/html
Date: Thu Jun 17 13:49:23 2021
Last-Modified: Thu Jun 17 13:45:01 2021
Server: CHESS
Content-Length: 196

<!doctype html>
<html>
	<head>
		<title>CHESS Templeton Landing Page</title>
		<meta name="description" content="Nothing special">
	</head>
	<body>
		CHESS Templeton Landing Page
	</body>
</html>
```

## Example interaction via stdin

```
echo -en 'GET /index.html HTTP/1.1\r\nHost: chess\r\n\r\n' | CHESS=chess ./build/templeton.bin -s
HTTP/1.1 200 OK
Content-Type: text/html
Date: Thu Jun 17 13:52:34 2021
Last-Modified: Thu Jun 17 13:45:01 2021
Server: CHESS
Content-Length: 196

<!doctype html>
<html>
	<head>
		<title>CHESS Templeton Landing Page</title>
		<meta name="description" content="Nothing special">
	</head>
	<body>
		CHESS Templeton Landing Page
	</body>
</html>
[INFO] Client disconnected
```

## Setup

The challenge should run as user chess.

The webroot for the server is /data. The initial files should be owned by user root and group chess with the permissions set to 644. This prevents a user from modifying the main server files.

# POV 1

CWE-94: Improper Control of Generation of Code
Code injection

The challenge fails to properly escape double quotations '"' when creating a php file which includes user input. This allows an attacker to inject arbitrary php code and leak the token file.

# POV 2

CWE-125: Out-of-bounds Read
Information Disclosure

The "Range" HTTP field allows a user to request subsets of a file. If an attacker requests a range that goes beyond the length of the file it is possible to leak heap data including addresses.

The way the allocations and frees work an attacker needs to request several ranges in order to leak an address.

'Range: bytes=0-1000,5-0,0-5,0-5\r\n'

The first range causes the leak while the second "5-0" is invalid and results in a free. This ensures that a heap address will be leaked since there will be a free block following the file data.