BITS 64

start:
	mov rax, 0x4080
	mov rbx, 0x40ff
	add ax, bx
	; rax = 0x817f
	; rbx = 0x40ff
	; flags: SF IF OF
	mov rax, -1
	add rax, 1
	; rax = 0
	; flags: CF PF AF ZF IF	
	mov rax, -1
	add eax, 1
	; rax = 0
	; flags: CF PF AF ZF IF
	mov rax, -1
	add ax, 1
	; rax = 0xffffffffffff0000
	; flags: CF PF AF ZF IF
	db 0xcc
