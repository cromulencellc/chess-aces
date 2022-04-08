# newbury

Newbury is based upon the open source project lighttpd. https://www.lighttpd.net

It uses the HTTP protocol so a good resource are the mozilla developer docs: https://developer.mozilla.org/en-US/docs/Web/HTTP

## Scope for Analysis

This service processes HTTP requests. The data under ``/data/webroot/homes`` is considered privileged data and requires a password to access. While the poller has access to these passwords via the ``/data/cheatsheet`` file, PoVs MUST NOT expect to have access to this information. Access to ``index.html`` is allowed for all users. PoVs MUST not expect the data to be persistent.

## Building

1. `./configure --prefix=/service/build`
2. `make && make install`

## Running

`/challenge/sbin/lighttpd -f /data/lighttpd.conf -D`

## Generating a data directory

You'll want ruby for this.

`ruby priv/filer.rb base_data` outputs a directory tree for `/data` into the `base_data` directory; you can pass in a different path for that second argument.

The poller and challenge both need this in `/data`

There's a `cheatsheet` file there that the poller uses but the challenge
doesn't. It includes passwords that are otherwise stored in hashes that a normal user would not have access to. This file is considered privileged data.

POV authors MUST NOT assume the existence or contents of this file, or knowledge of any of the passwords in it.

## POV 1

Authentification Bypass
Data Misuse

There is a use of unitialized data that occurs if an attacker supplies a password where the first byte is NULL.

## POV 2

Information Disclosure

If a user sets the Do Not Track header (DNT) then when the TRACKID portion of the cookie is set it is possible to leak a heap address since the code uses strlen() instead of sizeof() to specify the length of the string.

#ifndef PATCHED_2
	buffer_append_string_encoded_hex_lc(c.cookie, (char *)c.h, strlen((const char *)c.h));
#else
    buffer_append_string_encoded_hex_lc(c.cookie, (char *)c.h, sizeof(c.h));
#endif