# holt

Holt is based upon the open source project lighttpd. https://www.lighttpd.net

It uses the HTTP protocol so a good resource are the mozilla developer docs: https://developer.mozilla.org/en-US/docs/Web/HTTP

## Overview
Externality: Lighttpd 1.4.59

Protocol: HTTP

Language: C

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
CWE-78: Improper Neutralization of Special Elements used in an OS Command
Data injection

The usertrack module reads from the access log the last file that was requested and stores this information in the request log. This is done via a call to system so if the requested file contains back ticks then an attacker can run an arbitrary command on the host machine.

Evaluation of this PoV is done by comparing the returned token to that of the one stored in the filesystem of the challenge container. Since this does allow for command injection the effect does depend upon the PoV author.

## POV 2
CWE 674: Uncontrolled Recursion
Resource Consumption Error

The skeleton modules gives users the ability to auto fill HTTP request headers using data from the other headers. 

For example, if a user sends the following request

GET / HTTP/1.1
Host: challenge_container
Accept: */*
Some-Header: Hello {Accept} World

The skeleton module will replace {Accept} with the data contained in the field resulting in the following:

GET / HTTP/1.1
Host: challenge_container
Accept: */*
Some-Header: Hello */* World.

If an attacker creates a circular reference then, due to the way that the expansion happens, this will result in uncontrolled recursion. 
For example:

GET / HTTP/1.1
Host: challenge_container
Accept: */*
Some-Header: Hello {Some-Header} World.

The final resault will be a SEGFAULT due to consumption of the stack space but this is classified as a DOS due to resource consumption.
