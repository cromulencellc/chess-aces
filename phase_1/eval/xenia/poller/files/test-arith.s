.data

    shexstring: "0123456789ABCDEF"
    newline: "\n" db 0

.code

main:
    mov r0, NUM1
    mov r1, NUM2
    ARITH r0, r1

    call print_16bit_hex

    mov r0, newline
    call puts

    call exit

; exit() -- exit the process
exit:
    mov r0, 0
    mov r1, 0
    syscall



; putc(r0) -- Print the one-byte value in r0
putc:
    push r0
    mov r0, 2
    mov r1, sp
    mov r2, 1
    syscall
    pop r0
    ret
.data
    putc_buf: db 0
.code


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