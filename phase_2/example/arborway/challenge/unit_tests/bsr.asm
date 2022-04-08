BITS 64

start:
	xor rax, rax
	bsr rbx, rax
	; rbx: 0
	; flags: PF ZF IF

	mov rax, 0x555555554840
	mov rbx, -1
	bsr bx, ax
	; rbx: 0xffffffffffff000e
	; flags: IF

	mov rax, 0x555555554840
	push rax
	mov rbx, -1
	bsr bx, [rsp]
	pop rax
	; rbx: 0xffffffffffff000e
	; flags: IF
	
	mov rax, 0x555555554840
	mov rbx, -1
	bsr ebx, eax
	; rbx: 0x1e
	; flags: PF IF

	mov rax, 0x555555554840
	push rax
	mov rbx, -1
	bsr ebx, [rsp]
	pop rax
	; rbx: 0x1e
	; flags: PF IF

	mov rax, 0x555555554840
	mov rbx, -1
	bsr rbx, rax
	; rbx: 0x2e
	; flags: PF IF

	mov rax, 0x555555554840
	push rax
	mov rbx, -1
	bsr rbx, [rsp]
	pop rax
	; rbx: 0x2e
	; flags: PF IF
	
	db 0xcc
