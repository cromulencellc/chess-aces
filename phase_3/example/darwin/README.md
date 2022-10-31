# darwin

Darwin is based on the open source project proftpd. http://www.proftpd.org/

It uses the FTP protocol; https://en.wikipedia.org/wiki/File_Transfer_Protocol 

RFC959: https://datatracker.ietf.org/doc/html/rfc959

## Build
```
1. ./configure --with-modules=mod_ident --enable-dso --enable-ctrls --enable-nls
2. make && make install
3. Virtual user creation:
	a. sudo ftpasswd --passwd --file=/etc/proftpd/ftpd.passwd --name=username --uid=### --gid=### --home=/path/to/home --shell=/bin/false
	b. sudo ftpasswd --group --name=nogroup --file=/etc/proftpd/ftpd.group --gid=### --member username

```

## Run
```
sudo ./proftpd -n -c /etc/proftpd/proftpd.conf
```

## Running in Docker
To build & run challenge
```
docker-compose -f docker-compose.yaml build ta3_darwin
docker-compose up ta3_darwin
```
To build & run poller
```
docker-compose -f docker-compose.yaml build ta3_darwin_poller
docker-compose up ta3_darwin_poller
```
to build & run pov_x, where x is 1/2/3 (the number of the pov)
```
docker-compose -f docker-compose.yaml build ta3_darwin_pov_x
docker-compose up ta3_darwin_pov_x
```

## Scope for Analysis

This service processes ftp requests. Only the 'chess' user area is accessible via the 'chess' password. All other areas are considered privileged data and require passwords to access.
The poller accesses these areas using a cheatsheet, which includes passwords that are otherwise stored in hashes (in the ftpd.passwd file) that a normal user would not have access to. This file is considered privileged data. POV authors MUST NOT assume the existence or contents of this file, or knowledge of any of the passwords in it.
All non-chess users have a token file and a token file is provided in the / folder as well. The data contained in the token files are considered privileged data and should not normally be accessible.


## POV 1

```
Authentication Bypass/Structured Privileged Information Disclosure
Improper Input Validation (CWE-20) with Uninitialized variable (CWE-457) causes authentication bypass (CWE-305) leading to structured privileged information disclosure - token file (CWE-200).

If a user provides a valid username with a single null termination and extra characters(totaling 19 characters) with a 20th character that is 5, then it is possible to login without a proper password.

```

## POV 2

```
Information Disclosure
CWE-704/CWE-562/CWE-200

Using the STOR command with a large file (or never sending EOF) to the server allows the user to attempt a second command. If that second command is RETR, then the response has an incorrect type conversion of an address leading to return of a stack variable address; a memory address leak.

```

## POV 3

```
Information Disclosure
CWE-681/CWE-697/CWE-416 lead to CWE-125/CWE-200
Incorrect Conversion between Numeric Types with Incorrect Comparison and Use After Free  cause Out-of-Bounds Read and Exposure of Sensitive Information to an Unauthorized Actor

Using successive calls to ALLO will allow the user to leak a heap address. The user must make a directory with a name with a length of 127 characters and CWD to that directory. The user must then ALLO with a value causing a KB too large error, ALLO with -1, and finally ALLO with a "successful" value. The success message will then terminate with a memory address from the heap.

```
