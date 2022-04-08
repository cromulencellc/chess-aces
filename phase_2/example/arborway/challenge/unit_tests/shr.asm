BITS 64

start:
	mov rax, 0xdeadbeefcafebabe
	shr eax, 8	
	; rax = 0xcafeba
	; flags: CF IF OF
	mov rax, 0xdeadbeefcafebabe
	shr eax, 1
	; rax = 0x657f5d5f
	; flags: PF IF OF
	xor rax, rax
	shr rax, 4
	; flags: PF ZF IF
	db 0xcc
