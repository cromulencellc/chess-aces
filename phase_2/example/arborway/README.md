# arborway

This challenge is an x86_64 emulator. The user can provide a statically compiled elf file or just a string of bytes to be executed.

Arborway depends on capstone 4.0.2 which is provided. It also depends upon a uint256 library found here: https://github.com/calccrypto/uint256_t

## Running the challenge

The image must have a "chess" user.

There are two modes of execution for arborway. With the -s flag the challenge will run with input from stdin. Without this flag it will listen on a port and accept incoming connections.
5
It expects the PORT environment variable to be properly set.

If running with the -s flag, the -v and -b flag are also required. Use -v to specify the path to the vdso file and -b to specify the path to the elf binary.

If the challenge listens on a socket, once connected, a user will be given the following menu:

```
1) Send ELF
2) Send vdso
3) Execute
4) Exec SC
5) Quit
#
```

Option 1 prompts the user to send a file size followed by "size" bytes in 1000 byte chunks.
Option 2 is the same as option 1 but expects the vdso file.
Option 3 is only valid once the user has provided both an elf and vdso file
Option 4 allows a user to just supply an array of bytes to be executed
Option 5 exits the program.

Once the user is done uploading either the elf and the vdso or the shellcode they are brought to the emulation prompt which will look something like this:

```
[INFO] Executing now...
0# 0x41410000:  mov     eax, 0x3f   449
rax: 0x3f    rcx: 0x0    rdx: 0x0    rbx: 0x0
rsp: 0x7f0ffa8b2000  rbp: 0x0    rsi: 0x0    rdi: 0x0
rip: 0x41410005  r8: 0x0     r9: 0x0     r10: 0x0
r11: 0x0     r12: 0x0    r13: 0x0    r14: 0x0
r15: 0x0     IF

 _ This is the instruction number
|   _ Instruction address
|  |             _ Instruction mnemonic
|  |            |                    _ Internal instruction id
|  |            |                   |
0# 0x41410000:  mov     eax, 0x3f   449
```

At this prompt there are the following commands:

* `c` - Continue execution until finished
* `g <address>` - Execute until the emulator reaches a specific address
* `t <skip count>` - Execute *skip count* instructions then stop
* `set [pr | pe | pi]` - `pr` sets the print registers flag, `pe` sets the print extended registers flag, and `pi` sets the print instruction flag
* `unset [pr | pe | pi]` - `pr` turns off the print registers flag, `pe` turns off the print extended registers flag, and `pi` turns
                       off the print instruction flag
* `x [<address> | <reg>]` - print the data at the specified address or the address contained in reg
* `pm` - prints the currently allocated blocks of memory
* `q` - quits the program

# PoVs

## PoV 1

CWE-457: Use of uninitalized variable, 
CWE-20: Improper Input Validation

This combines several vulnerabilities including a leak of a stack address when executing an ELF due to an unitialized register. Under normal circumstances the emulator checks read/write addresses of the emulated application to ensure that the address is valid within its context to ensure that it cannot access the emulator's process space. However, this check does not take place when using the `fs` or `gs` segment registers. Access to these can be done by calls to the linux syscall `arch_prctl`.

By combining the leak in PoV 1, the memory access vulnerability with the `fs` registers, and the linux syscall, an attacker can leak an address of the emulator code. They can then use this leak along with another call to `arch_prctl` to leak the token.

## PoV 2

CWE-703: Improper Check or Handling of Exceptional Conditions,
CWE-755: Improper Handling of Exceptional Conditions,
CWE-209: Generation of Error Message Containing Sensitive Information,
CWE-390: Detection of Error Condition Without Action,
CWE-562: Return of Stack Variable Address

Causes a `FloatingPointException` with an `idiv` instruction and a 64 bit register. This results in the error number being set to a stack address and printed to the console. It is only accessible via uploaded shellcode.
