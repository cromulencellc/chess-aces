BITS 64

start:
	stc
	mov rbx, -1
	mov bx, 0x1280
	mov rcx, -1
	bsf cx, bx
	; rbx: 0xffffffffffff1280
	; rcx: 0xffffffffffff0007
	; flags: IF

	mov rbx, -1
	mov bx, 0x1280
	push rbx
	mov rcx, -1
	bsf cx, [rsp]
	; rbx: 0xffffffffffff1280
	; rcx: 0xffffffffffff0007
	; flags: IF

	mov rbx, -1
	mov ebx, 0x1280
	mov rcx, -1
	bsf ecx, ebx
	; rbx: 0x1280
	; rcx: 0x7
	; flags: IF

	mov rbx, -1
	mov ebx, 0x1280
	push rbx
	mov rcx, -1
	bsf ecx, [rsp]
	; rbx: 0x1280
	; rcx: 0x7
	; flags: IF

	mov rbx, 0x1280
	mov rcx, -1
	bsf rcx, rbx
	; rbx: 0x1280
	; rcx: 0x7
	; flags: IF
	
	mov rbx, 0x1280
	push rbx
	mov rcx, -1
	bsf rcx, [rsp]
	; rbx: 0x1280
	; rcx: 0x7
	; flags: IF

	xor rbx, rbx
	bsf rcx, rbx
	; PF ZF IF
	db 0xcc
