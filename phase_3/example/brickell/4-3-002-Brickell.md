#### Brickell

|||
|-:|:-|
|Externality|Lighttpd|
|Language|C|
|Protocol|HTTP|
|Weakness Categories|Input Validation, Memory Corruption|

Brickell is a web server based on the open source project lighttpd. The data in `/token` is considered privileged data and should not normally be accessible.

##### POV 1

* CWE-22: Improper Limitation of a Pathname to a Restricted Directory ('Path Traversal')
* CWE-20: Improper Input Validation

The challenge fails to prevent the user from specifying a user directory such as /~../. This leads to a path traversal to the root directory. The user is then able to obtain sensitive data in the form of the token file.

##### POV 2

* CWE-119: Improper Restriction of Operations within the Bounds of a Memory Buffer
* CWE-121: Stack-based Buffer Overflow

The challenge allows the user to corrupt the stack buffer with user defined code. The buffer overflow occurs when a request is made with a 'Sec-GPC' header and a value longer than 100 bytes. 
```python
'GET /index.html HTTP/1.1\r\n Host:' ...
    'Sec-GPC:' + '\x90'* 24 + '\x41' * 88 + '\x42' * 8 + '\x43' * 6
    ...
```
An errant copy that does not properly limit for the size of the destination buffer will overwrite the stack frame data following the buffer. If input is of a proper length then the RIP and RBP registers will be overwritten, potentially leading to arbitrary code execution. Entering random data of larger than 100 bytes will crash the server as well; normally it would just restart. 
