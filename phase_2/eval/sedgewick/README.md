# sedgewick

Sedgewick is an injection bug into the open source project proftpd. Specifically proftpd 1.3.7a.

It is an FTP server so instead of covering the entire protocol here I recommend reading the RFC:

https://tools.ietf.org/html/rfc959

or Wikipedia:

https://en.wikipedia.org/wiki/File_Transfer_Protocol

## Scope for Analysis

The FTP server is in scope. The data in ``/token`` is considered privileged data and neither the anonymous nor the chess accounts should have access. A PoV can assume access to both of these accounts but MUST NOT rely on the content of any of the available test files.

## Running 

Proftpd needs to be run with the following options:

``-n`` disable the background daemon.
``-X`` Don't fork
``-c`` Specify the configuration

```
/home/challenge/proftpd-1.3.7a/proftpd -X -n -c /home/challenge/proftpd-1.3.7a/sample-configurations/basic.conf
```

## Connecting

The easiest way to connect and communicate with the server is via an ftp client such as the default install of ``ftp``. For usage information for whichever client that is used check the man page.

There are two available accounts: anonymous:anon and chess:chess

## POVs

### PoV 1

Arithmetic Error
Information Disclosure

CWE-697: Incorrect Comparison
CWE-839: Numeric Range Comparison Without Minimum Check

The FTP protocol has the ability to restart a data transfer using the ``REST`` command. The argument to the command is the byte where the transfer should restart. If an attacker specifies a negative restart value then the server will send the data before the buffer which will likely contain addresses.

The PoV logs in as an anonymous user and sends the command ``REST -100000``. It then opens a port on which is can receive the data from the server. Finally it requests a valid file and the server sends the leaked data.

### PoV 2

Memory Corruptions

CWE-787 Out-of-bounds write

An attacker can send the ``OPTS`` command to modify the ``MLST`` parameters. If the attacker sends a buffer larger than 256 it is possible to overflow data on the stack. It could also be used to leak data but the PoV currently doesn't do that.

When the overflow occurs it overwrites the return address as well as the values of several saved registers.