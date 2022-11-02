## Grand

Grand is an injection vulnerability into the open source project Apache version 2.4.51. https://www.apache.org/

It uses the HTTP protocol so a good resource are the mozilla developer docs: https://developer.mozilla.org/en-US/docs/Web/HTTP


### Building

In the http-2.4.51 directory:
1. `./configure --prefix=/usr/local/apache2 --with-crypto --with-included-apr --with-mpm=prefork`
2.` make && make install`

In the http-2.4.51/mod_python directory
1. `./configure --with-python=/usr/bin/python3.8 --with-apxs=/usr/local/apache2/bin/apxs`
2. `make && make install`

### Running

`/usr/local/apache2/bin/apachectl`

### Poller

The base_data for this challenge consist of normally accessible data by anyone in folders - amsterdam through zurich, excluding webroot. 

There is a chess user with the password of chess to allow normal interaction with the server as a authenticated user. 

User data which is password protected is contained in a webroot folder in base_data; this is not normally accessible. These are provided for the poller and users to interact with.

There's a `cheatsheet` file there that the poller uses but the challenge doesn't. It includes passwords that are otherwise stored in hashes that a normal user would not have access to. This file is considered privileged data.

POV authors MUST NOT assume the existence or contents of this file, or knowledge of any of the passwords in it.

### PoV 1

* CWE-285: Improper Authorization 

Effect:     Authentication Bypass - Structured Privileged Information Disclosure

If a user adds an non-standard header `Setenv-Ifconfig: user`, the server calls the *add_setenvusercfg* function. This function mistakenly overwrites the username for a resource (in a configuration table), with the attacker supplied name. Therefore supplying a correct, active set of credentials will access a resource owed by another user. 

### PoV 2

* CWE-404/666: Resource Management
* CWE-668: Information Disclosure
* CWE-682/190: Arithmetic Error
* CWE-758: Logic Error

Effect:     Unstructured Privileged Information Disclosure

When a user performs an `OPTIONS` request via any url, the server stores the string of a non-standard limit/limitexcept in an allocation that is susequently freed. This string is the result of a *.htaccess* file being in use for that resource's directory. The string’s memory location is accessed whenever the `OPTIONS` request is repeated - a use after free.

An integer overflow vulnerability allows a user to set up where the string is located and have it potentially correspond to other users' login information. By making a follow-on requests (after a user has logged on with credentials), the attacker can gain usernames and password hashes.

The integer overflow is caused by a misrepresented version number of the calling agent. The program converts the agent version in the form of *x.x.x.x* into a decimal - anything not in this format is ignored. The issue is if the resulting value from the calculation exceeds the uint32 max, which will overflow into another uint32 (*leb_num*) in the struct. This *leb_num* is used to arbitrarily loop a str duplication, changing the location of the use-after-free string, from the `OPTIONS` request, on the heap.
### PoV 3

* CWE-74/95: Data/Code Injection

Effect:     Structured Privileged Information Disclosure

If a user adds the header `X-Redirect-By` to their HTTP request, and provides the value print('Hello Python')  while accessing py_echo.html, a print('HELLO') will be ran through a Server Side Include, and Hello Python will be printed on the server side console. What can be done is if you add semicolons after the print('Hello Python') given through the header, you can get that multiple lines of python code code all on one line. In doing this you can run arbitrary python code.

### PoV 4

* CWE-22: Path Traversal 

Effect:     Structured Privileged Information Disclosure

In the function ap_path_normalization, `%2E` url-encodings are not resolved and eliminated. Paired with using the cgi-bin which takes advantage of the `mod_alias` module, you are able to escape the Document Root.

### PoV 5

* CWE-400: Resource Management Error
* CWE-697: Logic Error

Effect:     Denial of Service due to Resource Exhaustion(Memory)

An additional module, `mod_speling`, is used. `mod_speling` provides the ability to correct a user's input to closely named resourses. If a user enters an invalid, single character directory preceded by a double slash, then the module will attempt to automatically correct the location to '.'. The module incorrectly indexes the location of the double slash and performs an incorrect comparison whether the character is a '.' or other single character. This creates the same path as was supplied; over which the function loops continually - as it never finds the correct or adequete path.

To verify the POV, access the challenge via terminal and view top. The httpd processes will spike to 100% CPU usage and climb in memory usage, because the allocator does not free resources until the request has finished. The OOM Killer will eventually kill the processes when they use all available memory.

