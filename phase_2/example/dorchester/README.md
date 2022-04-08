# Dorchester

This challenge injects vulnerabilities into the Mongoose Embedded Web Server

It was designed to be on the low end of difficulty.

The version used is provided as a zip file pulled from: https://github.com/cesanta/mongoose

This particular challenge starts a webserver that allows a client to login via a username and password form with the `login.html` page or create a new user via the signup.html page.

A secure session id is created after a valid username and password combination are provided so the client can then use that as a cookie to prove access privileges.

The `wwwroot` is set via the environment variable `WWWROOT` or defaults to `/var/www`.

## Example curl command to login

```
curl -vvv -X POST -d "user=chess" -d "pass=chess" <challenge_container>:8000/login.html
```

## Database

In addition to acting as a web server a user can read, write, and delete keys to a back end database via the PUT, GET, and DELETE http verbs to `<challenge_container>:8000/api/v1/<key>` with a variable `"value=<value>"`

To inject a variables into the database:

```
curl -vvv -X PUT -d "value=123" <challenge_container>:8000/api/v1/foo
curl -vvv -X PUT -d "value=success" <challenge_container>:8000/api/v1/bar/baz
```

To retrive existing variables:

```
curl -vvv <challenge_container>:8000/api/v1/bar/baz
```

To delete existing variables:

```
curl -vvv -X DELETE <challenge_container>:8000/api/v1/bar/baz
```

## Apogee Only

### POV 1

Access Control
CWE-253: Incorrect Check of Function Return Value
CWE-269: The software does not properly assign, modify, track, or check privileges for an actor, creating an unintended sphere of control for that actor.

When a client sends a POST to signup.html with a username and password, the server creates a user via a call to system with the useradd command. The username and password are sanitized so a command injection shouldn't be possible.

The server does not confirm that the useradd command succeeded. Failure could be the result of the  user already existing.

Following the useradd command, the server sets the password via chpasswd. However, if the user already exists then the password is being set to an attacker known value effectively giving them access to the account.

### POV 2

Memory Corruption
CWE-119: Improper Restriction of Operations within the Bounds of a Memory Buffer

If a clients POSTs a value that is greater than 0x50 bytes then a write beyond the bounds of memory can occur. If the buffer is large enough, an attack can overflow the return address as well as additional stored register values.

### POV 3

Memory Corruption & Access Control
CWE-119: Improper Restriction of Operations within the Bounds of a Memory Buffer

A user can write a value to the database using a PUT verb to the /api/v1 url. The user can also retrieve values that they have written to the database. When retrieving a value, a copy of it is stored in a session_info buffer in the global variables. If an attacker puts then retrieves a value that is longer than 128 bytes they can overflow the sessions structures in global memory. With this, they can forge a session cookie and login as if they had previously authenticated.

This exploit requires three connections. The first one puts the value in the database. The second retrieves the value and causes the overflow and the final is a when they actually connect to retrieve the webpage.