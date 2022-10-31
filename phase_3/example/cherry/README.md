# cherry

Cherry is an injection vulnerability into the open source CVS project version 1.12.23.

## Overview
Externality: CVS 1.12.23

Protocol: CVS

Language: C

## Running the server

You need fo have a cvs root directory which you can copy from base_data.

cvs -f --allow-root=\<path to cvsroot\>

It is likely that you will hae to run cvs as root.

## Checkout the repo

First you need to login

cvs -d :pserver:\<username\>@\<host\>:\<port\>/path/to/cvsroot login

You will be prompted with a password. For the docker image the user/pass combo is chess:chess

Now you can check out the repo

cvs -d :pserver:\<username\>@\<host\>:\<port\>/path/to/cvsroot checkout \<module\>

For the provided data you can use cvstesta

## Additional commands

For additional commands you can run:

cvs --help-commands

## Command help

For additional help and usage information for specific commands you can run:

cvs \<command\> --help-options

## In scope

The cvs server is in scope.

## POV 1
CWE-415: Double Free
CWE-123: Write-what-where condition
Memory Corruption, Arbitrary Write

If an attacker sends an invalid 'Directory' command twice with the same length argument this will result in freeing the buffer twice.

The following code was added to make exploitation slightly easier:

server.c

/* This is a placeholder for future development. Used to hook
   implement special argumentx parsing. */
void (*argument_vector_parse)(char *) = NULL;

serve_argumentx()
...
if (argument_vector_parse) {
argument_vector_parse(p);
}

## POV 2
CWE 674: Uncontrolled Recursion
Resource Consumption Error

If an attacker creates a symbolic link that references a parent of the link then executes the 'depth' command they can trigger an infinite recursion that will ultimately result in a SIGSEGV due to consumption of all the stack.

