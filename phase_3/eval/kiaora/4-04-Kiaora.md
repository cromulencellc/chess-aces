## Grand

Kiaora is an injection vulnerability into the open source project DikuMUD3. DikuMUD3 is an online multiuser dungeon(MUD) game server. It allows several users to interact together with/in a text based game world.

There are two executables the VME (Valhalla Mud Engine) and MPLEX (multiplexer). Although several multiplexers can be instantiated, this challenge uses only one in telnet mode. The MPLEX handles client connections and communication; while communicating with the VME via pipes. The VME maintains the state of the game and provides the information about that state through pipes to the MPLEX; to be sent back to the user. <br />

https://dikumud.com/ Source: https://github.com/Seifert69/DikuMUD3


### Building

In the DikuMUD3 directory:
1. `cmake .`
2. `make all -j8` # -j8 to compile on 8 threads in parallel
3. `make test` # optionally run the unit tests

### Running

From the DikuMUD3 directory
1. `cd ../bin/`
2. `./vme` # tab1
3. `./mplex -p 3040` # tab2

The mplex is intended as a telnet server for this challenge.

### Poller

The base_data is not maintained through subsequent uses of the challenge container. The files contained in the base_data folder are predefined users and their inventories for the purpose of this challenge. They are only modified internally in the challenge container and not stored externally.

The challenge is supplied with the user, 'chess' with a password of 'chess123'.

The poller for this challenge has 2 modes: automated tutorial follower and automated players. The poller will need to be re-built to change the selected options. In the poller_config.py file set the POLLER_MODE value to the approriate value, line 5.

Set POLLER_MODE: <br />
    1 = automated tutorial follower - takes over 22 minutes to complete. <br />
    2 = automated players - takes about 2.5 minutes to complete, when the NUMBER_OF_THREADS value is 3 and the DIRECTION_OPERATIONS value is 7.

##### By default the poller runs as automated players. 
The automated players option starts a number of users (by number of threads) who fail to login, successfully login, and then login and play for a while. If a thread choses an already playing character, then less threads will be performed. The number of operations the users perform is controlled by the DIRECTION_OPERATIONS variable in the poller_config.py file. The time for completion will vary by this count. The number of threads(number of players) can be changed via the NUMBER_OF_THREADS variable in the poller_config.py file.

##### Automated Tutorial
The poller will start a new character and start the tutorial. The poller reads for commands from the "newbie helper" and performs them. <br />
There a poller_cheatsheet.py file that the poller uses, knowledge of or contained in these files must not be assumed or known by any performers.

### PoV 1

* CWE-287: Authentication Error
* CWE-682: Arthmetic Error
* CWE-703/758/768: Logic Error

Effect:     Authentication Bypass

DikuMud3 VME uses a increasing time delay penalty for incorrect passwords. If, a user is logged in and the same account is attempted to be accessed with incorrect passwords 6 times, and then the orignal user logs out. A subsequent login attempt with an incorrect password will be successful.

The wait modifier is changed to 16, when a user is logged in and more than 4 login attempts have occured. This causes a signed int8_t negative value from an absolute value check (-128). This result is used as the first argument in a check where the password is also checked; thereby causing the password verification to be skipped. The default value (0) is used and is considered a successful login. This allows a unintented actor to access resources that do not belong to them.

This POV performs the previously stated actions: having a user logged in, attacker logging in incorrectly 6 times for that user, user logging out, and attacker successfully logging in.

### PoV 2

* CWE-74: Data/Code Injection
* CWE-170: Input Validation
* CWE-501: Data Misuse

Effect:     Private Address Disclosure

There is a format string vulnerability in the processing of an incorrect username. This can be used to leak a stack address. The vulnerability could be exploited to cause arbitrary code executions as well, but this POV only performs the information disclosure. The exploit is only possible when the user provides a string that begins with "(" and is at least 81 characters long. An incorrect check of what message is passed to the user including the extra "(" causes a boolean value to allow an off-by-one overwrite of the 81st character input to a second boolean value. The combination of controlling both boolean values allows the format string vulnerability value to be returned to the user in a message.
```python
username = '(' + 'a'*79+ ' %17$p '
```
The POV sends the message and retrieves the address disclosed.

### PoV 3

* CWE-118: Memory Corruption
* CWE-138/170: Input Validation

Effect:     Register Control/Memory Corruption

This POV may not always succeed. The POV requires 3 addresses without bytes values below 32(*0x20*). However, stopping and restarting the challenge will allow for a new set of memory addresses; the POV usually succeeds within 3 tries.

There is a memory corruption vulnerability that overwrites a function pointer in the class *cConHook* object for a user. The exploit for this POV leverages the address leak in POV2. After obtaining a libc address leak, the user must be logged in.

1. A long message (1025 bytes) specifically ending in an escape sequence of `\x1b5b4b` (*esc + [ + M*), creates a 1 byte overwrite of with the value of 2. This value is overwritten into the lowest byte of the *max_input_len* variable, causing the next read to be 2 bytes longer (1026 bytes).
2. Next the overwrite is extended to larger value, however overread is capped at 1040 bytes by a different check.
3. There is protection to prevent any characters not within 32(*0x20*) and 127(*0x7f*) for a telnet connection. This would prevent using characters necessary to pass addresses for *RIP* control. The telnet protection is removed with the overwrite and a bad character effectively changing the server to webserver, which allows character values of 32 to 255.
4. Finally, the *m_pFptr* function pointer can be overwritten. This function pointer is called after the character checking/overwriting has occured.
    a. A stack pivot is necessary to gain control of *RIP*. The value of the original input buffer can placed in *RDX* with an extra escape sequence overwrite after the function pointer.  </br>
    b. There is a single ROP gadget in libc2.31 for `mov rsp, rdx;ret;`. This gadget can be used to move the stack to the user control buffer with another gadget for register control. 
The POV performs the above steps. It calculates the addresses needed first and if any have bytes that will not work, then the POV fails with a message to restart the challenge and try again.

### PoV 4

* CWE-400/674 Resource Management Error

Effect:     Denial of Service due to Resource Exhaustion (Stack)

There is an alias system in DikuMUD that allows users to create their own commands or composite commands. There is protection to prevent circular aliases. However, there is an additional command `!` that allows a user to enter the previous command. Therefore, an attacker can create an alias that uses the `!` and call it. This will cause an infinite loop leading to the stack exhaustion.
