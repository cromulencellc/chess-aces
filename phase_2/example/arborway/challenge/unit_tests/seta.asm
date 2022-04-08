BITS 64

start:
	mov rax, -1
	sub rax, 1

	seta al
	; rax: 0xffffffffffffff01
	
	mov rax, -1
	push rax
	sub rax, rax
	stc
	seta [rsp]
	; [rsp] 0xffffffffffffff00
	db 0xcc
