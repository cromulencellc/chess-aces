# Grand
This README includes details of challenge internals and vulnerabilities. It should not be packaged for distribution to evaluation participants.

**Do not distribute this README to the control team or CHESS teams during the evaluation event.**

Grand is an injection vulnerability into the open source project Apache version 2.4.51. https://www.apache.org/

# Overview
Externality: apache<br />
Protocol: HTTP<br />
Language: C<br />

## Scope for Analysis
All code included in this challenge is in scope. The data contained in /token is considered privileged data and should not normally be accessible. There is also a token file in each of the specific user folders (alpha through zulu), which are password protected - this is considered privileged data and should not normally be accessible. There is a chess user with the password of chess to allow normal interaction with the server as a authenticated user. 

## Base Data
The base_data for this challenge consist of normally accessible data by anyone in folders - amsterdam through zurich, excluding webroot. User data which is password protected is contained in a webroot folder in base_data; this is not normally accessible. 

## Running in Docker
To build & run challenge
```
docker-compose -f docker-compose.yaml build ta3_grand
docker-compose up ta3_grand
```
To build & run poller
```
docker-compose -f docker-compose.yaml build ta3_grand_poller
docker-compose up ta3_grand_poller
```
to build & run pov_x, where x is the number of the pov
```
docker-compose -f docker-compose.yaml build ta3_grand_pov_x
docker-compose up ta3_grand_pov_x
```

## Build Apache
```
In the http-2.4.51 directory:
1. ./configure --prefix=/usr/local/apache2 --with-crypto --with-included-apr --with-mpm=prefork
2. make && make install
In the http-2.4.51/mod_python directory
1. ./configure --with-python=/usr/bin/python3.8 --with-apxs=/usr/local/apache2/bin/apxs
2. make && make install
```

## Running Apache
```
/usr/local/apache2/bin/apachectl
```

## Poller
The poller runs several common tasks for accessing the webserver. It runs a random number of actions and picks random actions by random users. <br />
There's a cheatsheet file containing the usernames and passwords for accounts on the database, knowledge of or contained in these files must not be assumed or known by any performers. <br />
Apache only has two processes; limiting the number of simultaneous connections. The number of tests has been limited to allow the poller to complete in a reasonable amount of time.

# POVs Apogee Only
Weakness Categories: access control error, resource management error, information disclosure, arithmetic error, logic error, data/code injection, path traversal

## POV 1
Weakness:   CWE-287: Access Control Error <br />
Effect:     Authentication Bypass <br />
If a user adds an non-standard header "Setenv-Ifconfig: user", then a mismanagement of the configuration filename cause the user supplied username to be used. The configuration filename should be used for the folder being accessed. However, with this vulnerability a user can supply a different set of valid credentials and access resources that do not belong to them. This causes an access control vulnerabilty: software incorrectly performs an authorization check when an actor attempts to access a resource. In this POV, the token file in the user "tango" folder is retrieved using the chess:chess credentials.

## POV 2
Weakness:   CWE-404/666: Resource Management Error, CWE-668: Information Disclosure, CWE-682/190: Arithmetic Error, CWE-758: Logic Error <br />
Effect:     Unstructured Privileged Information Disclosure <br />
When a user performs an OPTIONS request via any url, the server stores the string of a non-standard limit/limitexcept in an allocation that is susequently freed. The string’s memory location is accessed whenever the OPTIONS request is repeated. <br />
An integer overflow vulnerability allows a user to set up where the string is located and have it potentially correspond to other users' login information. By making a follow-on request (after a user has logged on with credentials), the attacker can gain usernames and password hashes. <br />
The integer overflow is caused by a misrepresented version number of the calling agent. The program converts the agent version in the form of x.x.x.x into a decimal - anything not in this format is ignored. The issue is if the resulting value from the calculation exceeds the uint32 max, which will overflow into another uint32 (leb_num) in the struct. This leb_num is used to arbitrarily loop a str duplication, changing the location of the use-after-free string on the heap. <br />
There are several conditions, which must be aligned to retrieve the hash. The POV works best when it is run first after starting the challenge container. The POV is possible after the poller is run, but may take several attempts to find a valid hash. <br />

## POV 3
Weakness:   CWE-74/95: Data/Code Injection <br />
Effect:     Structured Privileged Information Disclosure <br />
If a user adds the header X-Redirect-By to their HTTP request, and provides the value print('Hello Python')  while accessing py_echo.html, a print('HELLO') will be ran through a Server Side Include, and Hello Python will be printed on the server side console. What can be done is if you add semicolons after the print('Hello Python') given through the header, you can get that multiple lines of python code code all on one line. In doing this you can run arbitrary python code.

## POV 4
Weakness:   CWE-22: Path Traversal <br />
Effect:     Structured Privileged Information Disclosure <br />
In the function ap_path_normalization, %2E url-encodings are not resolved and eliminated. Paired with using the cgi-bin which takes advantage of the mod_alias module, you are able to escape the Document Root.

## POV 5
Weakness:   CWE-400: Resource Management Error, CWE-697 Logic Error <br />
Effect:     Denial of Service due to Resource Exhaustion(Memory)  <br />
An additional module, mod_speling, is used. mod_speling provides the ability to correct a user's input to closely named resourses. If a user enters an unknown single character directory preceded by a double slash, then the module will attempt to automatically correct the location to '.'. The module incorrectly indexes the location of the double slash and performs an incorrect comparison whether the character is a '.' or other single character. This creates the same path as was supplied; overwhich the function loops continually - as it never finds the correct path. <br  />
To verify the POV, access the challenge via terminal and view top. The httpd processes will spike to 100% CPU usage and climb in memory usage, because the allocator does not free resources until the request has finished. The OOM Killer will eventually kill the processes when they use all available memory.
