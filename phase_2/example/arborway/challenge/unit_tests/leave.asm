BITS 64

start:
	mov rax, 0xb00bd00db00bd00d
	push rax
	mov rax, 0xdeadbeefcafebabe
	push rax

	mov rbp, rsp
	sub rsp, 0x40

	leave
	ret
	;[rsp] 0xb00bd00db00bd00d
	; rbp: 0xdeadbeefcafebabe
	db 0xcc
