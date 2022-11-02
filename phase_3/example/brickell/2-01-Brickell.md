## Brickell

Brickell is based upon the open source project lighttpd. https://www.lighttpd.net

It uses the HTTP protocol so a good resource are the mozilla developer docs: https://developer.mozilla.org/en-US/docs/Web/HTTP


### Building

1. `./configure`
2. `make && make install`

### Running

`/challenge/sbin/lighttpd -f /data/lighttpd.conf -D`

### Poller

The base_data contains password protected user data in folders alpha through zulu. These are provided for the poller and users to interact with. The configuration data files are necessary for the challenge to operate.
There's a `cheatsheet` file there that the poller uses but the challenge doesn't. It includes passwords that are otherwise stored in hashes that a normal user would not have access to. This file is considered privileged data.

POV authors MUST NOT assume the existence or contents of this file, or knowledge of any of the passwords in it.

### POV 1

* CWE-22: Improper Limitation of a Pathname to a Restricted Directory ('Path Traversal')
* CWE-20: Improper Input Validation

Effect:     Structured Privileged Information Disclosure

The challenge fails to prevent the user from specifying a user directory such as /~../. This leads to a path traversal to the root directory. The user is then able to obtain sensitive data in the form of the token file.

### POV 2

* CWE-119: Improper Restriction of Operations within the Bounds of a Memory Buffer
* CWE-121: Stack-based Buffer Overflow

Effect:     Memory Corruption

The challenge allows the user to corrupt the stack buffer with user defined code. The buffer overflow occurs when a request is made with a 'Sec-GPC' header and a value longer than 100 bytes. 
```python
'GET /index.html HTTP/1.1\r\n Host:' ...
    'Sec-GPC:' + '\x90'* 24 + '\x41' * 88 + '\x42' * 8 + '\x43' * 6
    ...
```
An errant copy that does not properly limit for the size of the destination buffer will overwrite the stack frame data following the buffer. If input is of a proper length then the RIP and RBP registers will be overwritten, potentially leading to arbitrary code execution. Entering random data of larger than 100 bytes will crash the server as well; normally it would just restart. 
