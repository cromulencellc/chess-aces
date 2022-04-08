# Fenway

## What is Fenway?
Fenway is a custom Server Message Block (SMB) server built with the intention of hosting a 
whiteboard application.

In practice this means creating, editing, and hosting image files both in black and white,
and color images with PGM and PPM files. The SMB server facilitates this by giving
a language for the server and client to communicate. The client sends a message, the
server understands provided the client has communicated the message correctly, the server 
services the request and sends back a response.

## Running the Challenge
`docker-compose -f docker-compose.yaml up ta3_fenway`


## Poller
The poller will execute all of the possible SMB commands supported on the server
including commands crafted to hit errors and will check to make sure that
the appropriate error messages are communicated in the response. At the end of execution
it gives a report on if the intended functionality is working.

To run the poller do the following command:
`docker-compose -f docker-compose.yaml up ta3_fenway_poller`

## POV
The bug in Fenway is a memory corruption bug. This is very important to keep in mind. There is 
no private information on Fenway, thus no structured or unstructured information disclosures.
Additionally there is no token to be taken, the bug is the memory corruption.

This takes the form of a CWE-666 Operation on Resource in Wrong Phase of Lifetime. The exploit
takes advantage of the fact that when making an allocation on the heap, and then freeing it, and
asking for another allocation of a similar size it is common for the heap to return the the block 
that you just freed. Given this if the previous allocation is not set to NULL, you have
a dangling pointer to your old struct still in memory. This is exactly whats happens in Fenway.
A ppm file is made through a CREATE_NEW request and the ppm struct in memory fails to be NULL'd.
This results in the ability to after freeing the struct use it again by way of a calling a 
function using the previously freed struct's function pointer member variable processing of request. 
Fenway checks the file extension to see which function is to be called. This ends with an attacker 
being able to craft a CREATE command with a ppm extension after the intial CREATE_NEW request which 
calls the function pointed to by the function pointer that was freed earlier causing a crash, 
and resulting in control of two registers.

## Running the POV
When running the to test the POV server you will want to run the POV version of 
the server with:
`docker-compose -f docker-compose.yaml up ta3_fenway_pov_server`

This allows gdb to show the crash and display the values of the relevant registers
at the time of the crash. Once the pov server is up, run the command below to run
the actual pov script.

`docker-compose -f docker-compose.yaml up ta3_fenway_pov`

## Reference Patch
The patch for this vulnerability is to make sure that when using dynamically allocated memory
that you free and NULL out the pointer preventing the struct from being able to be referenced
at all. This action is taken in the reference patch inside the destroy_pgm and destroy_ppm functions.

## Running the Reference Patch Version
`docker-compose -f docker-compose.yaml up ta3_fenway_ref_patch`

