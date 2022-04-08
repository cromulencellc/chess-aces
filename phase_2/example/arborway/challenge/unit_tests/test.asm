BITS 64

start:
	mov rax, -1
	test al, al
	; flags: PF SF IF
	mov rbx, 0
	test rbx, rbx
	; flags: PF ZF IF
	mov rax, 0x1f0
	mov rbx, 0x280
	test rax, rbx
	; flags: IF
	test rax, rax
	; flags: PF IF
	db 0xcc
