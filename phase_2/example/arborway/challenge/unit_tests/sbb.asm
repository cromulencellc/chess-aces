BITS 64

start:
	xor rax, rax
	xor rax, rax

	mov rax, 1
	xor rax, 0

	mov rax, -1
	mov ax, 0x2
	stc
	sbb ax, 1
	; rax: 0xffffffffffff0000
	; flags: PF ZF IF

	mov rax, -1
	mov al, 20
	stc
	sbb al, 21
	; rax: 0xfffffffffffffffe
	; flags: CF AF SF IF

	mov rax, -1
	mov al, 20
	clc
	sbb al, 21
	; 0xffffffffffffffff
	; flags: CF PF AF SF IF

	mov rax, -1
	mov ah, 20
	stc
	sbb ah, 21
	; rax: 0xfffffffffffffeff
	; flags: CF PF AF SF IF

	mov rax, -1
	mov ax, 20
	stc
	sbb ax, 21
	; rax: 0xfffffffffffffffe
	; flags: CF AF SF IF

	mov rax, -1
	mov ax, 20
	clc
	sbb ax, 21
	; rax: 0xffffffffffffffff
	; flags: CF PF AF SF IF

	mov rax, -1
	mov eax, 20
	stc
	sbb eax, 21
	; rax: 0xfffffffe
	; flags: CF AF SF IF

	mov rax, -1
	mov eax, 20
	clc
	sbb eax, 21
	; rax: 0xffffffff
	; flags: CF PF AF SF IF

	mov rax, 20
	stc
	sbb rax, 21
	; rax: 0xfffffffffffffffe
	; flags: CF AF SF IF

	mov rax, 20
	clc
	sbb rax, 21
	; rax: 0xffffffffffffffff
	; flags: CF PF AF SF IF
	
	db 0xcc
