# Kiaora
This README includes details of challenge internals and vulnerabilities. It should not be packaged for distribution to evaluation participants.

**Do not distribute this README to the control team or CHESS teams during the evaluation event.**

Kiaora is an injection vulnerability into the open source project DikuMUD3. DikuMUD3 is an online multiuser dungeon(MUD) game server. It allows several users to interact together with/in a text based game world. There are two executables the VME (Valhalla Mud Engine) and MPLEX (multiplexer). Although several multiplexers can be instantiated, this challenge uses only one in telnet mode. The MPLEX handles client connections and communication; while communicating with the VME via pipes. The VME maintains the state of the game and provides the information about that state through pipes to the MPLEX; to be sent back to the user. <br />

https://dikumud.com/ Source: https://github.com/Seifert69/DikuMUD3

# Overview
Externality: none<br />
Protocol: TCP/IP<br />
Language: C<br />

## Scope for Analysis
Only the VME and MPLEX binaries are in scope. The other binaries (vmc/pp/defcomp etc.) are not in scope for this challenge. The data contained in /token is considered privileged data and should not normally be accessible. All code included in challenge/vme/src is in scope. The data contained in /token is considered privileged data and should not normally be accessible. The user data for the included users is in the /base_data/ply/ directory. There is a chess user with the password chess123 to allow normal interactions with the server. All users have passwords, included for the poller in the poller_cheatsheet.py file. This file is considered privileged data POV authors MUST NOT assume the existence or contents of this file, or knowledge of any of the passwords in it.

## Base Data
The data is not maintained through subsequent uses of the challenge container. The files contained in the base_data folder are predefined users and their inventories for the purpose of this challenge. They are only modified internally in the challenge container and not stored externally.

## Running in Docker
To build & run challenge
```
docker-compose -f docker-compose.yaml build ta3_kiaora
docker-compose up ta3_kiaora

Re-build and remove player history:
docker-compose -f docker-compose.yaml build --no-cache ta3_kiaora

Reference-patch:
Ensure the container is re-built when switching between the challenge and reference-patch.
docker-compose -f docker-compose.yaml build ta3_kiaora_ref_patch
docker-compose up ta3_kiaora_ref_patch
```
To build & run poller
```
docker-compose -f docker-compose.yaml build ta3_kiaora_poller
docker-compose up ta3_kiaora_poller
```
to build & run pov_x, where x is 1, 2, 3 (the number of the pov)
```
docker-compose -f docker-compose.yaml build ta3_kiaora_pov_x
docker-compose up ta3_kiaora_pov_x
```

## Building DikuMUD3
In the DikuMUD3 directory:
```
1. cmake .
2. make all -j8 # -j8 to compile on 8 threads in parallel
3. make test # optionally run the unit tests
```

## Running DikuMUD3
From the DikuMUD3 directory
```
cd ../bin/
./vme           #tab1
./mplex -p 3040 #tab2
```
The mplex is intended as a telnet server for this challenge.

## Poller
The challenge is supplied with the user, 'chess' with a password of 'chess123'. <br />
The poller for this challenge has 2 modes: automated tutorial follower and automated players. The poller will need to be re-built to change the selected options. In the poller_config.py file set the POLLER_MODE value to the approriate value, line 5.  <br />
Set POLLER_MODE: <br />
    1 = automated tutorial follower - takes over 22 minutes to complete. <br />
    2 = automated players - takes about 2.5 minutes to complete, when the NUMBER_OF_THREADS value is 3 and the DIRECTION_OPERATIONS value is 7.

### By default the poller runs as automated players. 
The automated players option starts a number of users (by number of threads) who fail to login, successfully login, and then login and play for a while. If a thread choses an already playing character, then less threads will be performed. The number of operations the users perform is controlled by the DIRECTION_OPERATIONS variable in the poller_config.py file. The time for completion will vary by this count. The number of threads(number of players) can be changed via the NUMBER_OF_THREADS variable in the poller_config.py file.

### Automated Tutorial
The poller will start a new character and start the tutorial. The poller reads for commands from the "newbie helper" and performs them. <br />
There a poller_cheatsheet.py file that the poller uses, knowledge of or contained in these files must not be assumed or known by any performers.

# POVs Apogee Only
Weakness Categories: authentication error, data/code injection, input validation, data misuse, memory corruption, resource management error

## POV 1
Weakness:   CWE-287: Authentication Error, CWE-682: Arthmetic Error, CWE-703/758/768: Logic Error<br />
Effect:     Authentication Bypass<br />
DikuMud3 VME uses a increasing time delay penalty for incorrect passwords. If, a user is logged in and the same account is attempted to be accessed with incorrect passwords 6 times, and then the orignal user logs out. A subsequent login attempt with an incorrect password will be successful. <br />
The wait modifier is changed to 16, when a user is logged in and more than 4 login attempts have occured. This causes a signed int8_t negative value from an absolute value check (-128). This result is used as the first argument in a check where the password is also checked; thereby causing the password verification to be skipped. The default value (0) is used and is considered a successful login. This allows a unintented actor to access resources that do not belong to them.  <br />
This POV performs the previously stated actions: having a user logged in, attacker logging in incorrectly 6 times for that user, user logging out, and attacker successfully logging in.

## POV 2
Weakness:   CWE-74: Data/Code Injection, CWE-170: Input Validation, CWE-501: Data Misuse, CWE-562/134/484
Effect:     Private Address Disclosure <br />
There is a format string vulnerability in the processing of an incorrect username. This can be used to leak a stack address. The vulnerability could be exploited to cause arbitrary code executions as well, but this POV only performs the information disclosure. The exploit is only possible when the user provides a string that begins with "(" and is at least 81 characters long. An incorrect check of what message is passed to the user including the extra "(" causes a boolean value to allow an off-by-one overwrite of the 81st character input to a second boolean value. The combination of controlling both boolean values allows the format string vulnerability value to be returned to the user in a message. <br />
The POV send the message and retrieves the address disclosed.

## POV 3
Weakness:   CWE-118: Memory Corruption, CWE-138/170: Input Validation <br />
Effect:     Register Control/Memory Corruption <br />
This POV may not always succeed. The POV requires 3 addresses without bytes values below 32(0x20). However, stopping and restarting the challenge will allow for a new set of memory addresses; the POV usually succeeds within 3 tries. <br />
There is a memory corruption vulnerability that overwrites a function pointer in the class cConHook object for a user. The exploit for this POV leverages the address leak in POV2. After obtaining a libc address leak, the user must be logged in. <br />
1. A long message specifically ending in an escape sequence of "\x1b5b4b" (esc + [ + M), creates a 1 byte overwrite of with the value of 2. This value is overwritten into the lowest byte of the max_input_len variable, causing the next read to be 2 bytes longer (1026 bytes).  <br />
2. Next the overwrite is extended to larger value, however overread is capped at 1040 bytes by a different check.  <br />
3. There is protection to prevent any characters not within 32 (0x20) and 127(0x7f) for a telnet connection. This would prevent using characters necessary to pass addresses for RIP control. The telnet protection is removed with the overwrite and a bad character effectively changing the server to webserver, which allows character values of 32 to 255.  <br />
4. Finally, the m_pFptr function pointer can be overwritten. This function pointer is called after the character checking/overwriting has occured.  <br />
    a. A stack pivot is necessary to gain control of RIP. The value of the original input buffer can placed in RDX with an extra escape sequence overwrite after the function pointer.  <br />
    b. There is a single ROP gadget in libc2.31 for "mov rsp, rdx;ret;". This gadget can be used to move the stack to the user control buffer with another gadget for register control. <br />
The POV performs the above steps. It calculates the addresses needed first and if any have bytes that will not work, then the POV fails with a message to restart the challenge and try again.

## POV 4
Weakness:   CWE-400/674 Resource Management Error
Effect:     Denial of Service due to Resource Exhaustion(Stack) <br />
There is an alias system in DikuMUD that allows users to create their own commands or composite commands. There is protection to prevent circular aliases. However, there is an additional command '!' that allows a user to enter the previous command. Therefore, an attacker can create an alias that uses the '!' and call it. This will cause an infinite loop leading to the stack exhaustion. <br />
The POV creates an alias for '!' and calls it.<br />
