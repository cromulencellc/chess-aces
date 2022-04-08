BITS 64

start:
	mov rax, 0xdeadbeefcafebabe
	mov rbx, 0xd00dd00db00bb00b
	and eax, ebx
	; rax = 0x800ab00a
	; flags: PF SF IF
	xor rax, rax
	xor rbx, rbx
	and rax, rbx
	; flags: PF ZF IF

	mov al, 0xff
	mov bl, 0x80
	and al, bl
	; rax = 0x80
	; flags: SF IF
	;
	db 0xcc
