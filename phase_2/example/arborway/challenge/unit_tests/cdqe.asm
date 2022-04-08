BITS 64

start:
	mov rax, 0xffffffff00000020
	cdqe
	; rax: 0x20

	mov rax, 0x80000010
	cdqe
	; rax: 0xffffffff80000010	
	db 0xcc
