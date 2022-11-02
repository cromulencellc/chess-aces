## Darwin

Darwin is based on the open source project proftpd. This service processes ftp requests. It uses the FTP protocol; https://en.wikipedia.org/wiki/File_Transfer_Protocol 
RFC959: https://datatracker.ietf.org/doc/html/rfc959


### Building

1. `./configure --with-modules=mod_ident --enable-dso --enable-ctrls --enable-nls`
2. `make && make install`
3. Virtual user creation:
	a. `sudo ftpasswd --passwd --file=/etc/proftpd/ftpd.passwd --name=username --uid=### --gid=### --home=/path/to/home --shell=/bin/false`
	b. `sudo ftpasswd --group --name=nogroup --file=/etc/proftpd/ftpd.group --gid=### --member username`

### Running

`sudo ./proftpd -n -c /etc/proftpd/proftpd.conf`

### Poller

The base_data contains password protected user data. These are provided for the poller and users to interact with.
There's a `cheatsheet` file there that the poller uses but the challenge doesn't. It includes passwords that are otherwise stored in hashes that a normal user would not have access to. This file is considered privileged data.

POV authors MUST NOT assume the existence or contents of this file, or knowledge of any of the passwords in it.

### PoV 1

* CWE-20: Improper Input Validation
* CWE-457: Uninitialized Variable
* CWE-305: Authentication Bypass

Effect:     Authentication Bypass - Structured Privileged Information Disclosure

If a user provides a valid username with a single null termination and extra characters(totaling 19 characters) with a 20th character that is 5, then it is possible to login without a proper password. The challenge incorrectly parses the username input for a double null character. Therefore, an input with one null will cause additional data in the buffer to be passed through the program.
In the `auth_chkpass` function, the return value `res` is not initialized. The result is the excess data may be used versus an initialized value, because it occupies the same location. If the value specifically in the uninitialized `res` is 5, equivalent to `CRYPT_PASS_SUCCESS` enum, then the authentication passes regardless of the value of the password.

### PoV 2

* CWE-704: Incorrect Type Conversion (Data Misuse)
* CWE-668/CWE-562: Information Disclosure/Return of Stack Variable Address

Effect:    Unstructured Privileged Information Disclosure

Using the `STOR` command with a large file (or never sending EOF) allows the user to attempt a second command while the server is processing the first. If that second command is `RETR`, then there is an error in the response. An incorrect type conversion of an address of a stack variable is returned to the user in the response.

### PoV 3

* CWE-691: Logic Error (CWE-681/697: Incorrect Conversion between Numeric Types/Incorrect Comparison)
* CWE-668/200: Information Disclosure
* CWE-125: Out of Bounds Read

Effect:     Unstructured Privileged Information Disclosure

Using successive calls to `ALLO` will allow the user to leak a heap address. The user must make a directory with a name with a length of 127 characters and `CWD` to that directory. The user must then `ALLO` with a value causing a KB too large error, `ALLO` with -1, and finally `ALLO` with an 'in-range' value. The success message returned to the user will then terminate with a memory address from the heap.

```python
   ftp.send('ALLO 4398046511104\r\n'.encode('utf-8'))
   ftp.send('ALLO -1\r\n'.encode('utf-8'))
   ftp.send('ALLO 1234\r\n'.encode('utf-8'))
```
