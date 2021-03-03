.data
    beat_navy: "Beat Navy!\n" db 0

.code

    mov r0, 2
    mov r1, beat_navy
    mov r2, 11
    syscall

    mov r0, 0
    syscall
END_PROGRAM