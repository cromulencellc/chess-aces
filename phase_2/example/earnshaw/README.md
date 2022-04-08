# earnshaw

## Overview

Earnshaw is a modifiction bug based on the open source project vsftpd. The specific version used here is 3.0.3 and can be obtained from: https://security.appspot.com/downloads/vsftpd-3.0.3.tar.gz
It is a full featured FTP server which implements most of the commands covered here: https://en.wikipedia.org/wiki/List_of_FTP_commands

The bugs in earnshaw are not based upon configuration issues. The way vsftpd is configured is not secure, it is only done in the easiest possible way to make the challenge work. THe only username and password combination that you have is anonymous:anon. While this is technically an unprivileged account, the configuration gives it all the privileges of root. Don't make a POV that logs in as anonymous and submit it saying that you have root access.

## Setup

If launching outside of the docker image you need to create a user:
useradd -ms /bin/bash anon
echo anon:anon | chpasswd

To compile you need two additional packages: libwrap0-dev libssl-dev

## Launch

Running the server is simple. As root or with sudo run with: 'vsftpd vsftpd.conf'

## POVs Apogee Only

### POV 1

CWE-305: Authentication Bypass by Primary Weakness

The vsftpd server authenticates a password by first calling getspnam() to retrieve the crypt'd password of the user. It then passes the provided password from the user via the PASS verb to the crypt() function. The returned valued is then compared with the value from getspnam(). If they are identical then authentication succeeds otherwise, it fails. 

Here is an example crypt'd password: $6$Rq4YcdTO$kUKezj97DD01kU4BFFIUs.yu7fXfyvIwobMoDXt0a0BKCehtA5GCpA0cl3moI2wKhPKyRRUusUpdFNoT1KdiS.

The bug removes the call to crypt() so the comparison is actually done only on the raw value sent via the PASS command and not the crypted version. It also uses strncmp() as a comparison function based upon the length of the value sent by the user. So, if the user sends only the value '$' the strncmp() will return 0 since all crypt() values begin with a $.

### POV 2

CWE-122: Heap-based Buffer Overflow

The NLST command acts mostly like the linux shell command 'ls' including the ability to filter results using tokens such as '*' or '?'.

If the user sends the NLST command with a filter then they can cause a heap-bases buffer overflow if the filter is longer than 16 bytes.

### POV 3

CWE-704: Incorrect Type Conversion or Cast

The MDTM command gives the last modification time of a file. If the file does not exist then there is a bug in the casting of a pointer point (**)
to a string pointer and printing the data there. This results in an information disclosure.


