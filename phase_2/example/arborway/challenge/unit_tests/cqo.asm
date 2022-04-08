BITS 64

start:	
	mov rax, 0xdeadbeefcafebabe
	xor rdx, rdx
	cqo
	; rax: 0xdeadbeefcafebabe
	; rdx: -1

	mov rax, 0x0eadbeefcafebabe
	cqo
	; rdx: 0	
	db 0xcc
