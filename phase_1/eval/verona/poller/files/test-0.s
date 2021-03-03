.data

    success_str: "SUCCESS\n" db 0
    failure_str: "FAILURE\n" db 0

.code

main:
    mov r0, 1
    mov r1, 2
    mov r2, 3
    mov r3, 5
    mov r4, 7
    mov r5, 11
    mov r6, 13
    mov r7, 17

    add r0, r1
    add r0, r2
    add r0, r3
    add r0, r4
    add r0, r5
    add r0, r6
    add r0, r7

    cmp r0, 59
    je success

    mov r0, failure_str
    call puts
    call exit

success:
    mov r0, success_str
    call puts
    call exit

; exit() -- exit the process
exit:
    mov r0, 0
    mov r1, 0
    syscall

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