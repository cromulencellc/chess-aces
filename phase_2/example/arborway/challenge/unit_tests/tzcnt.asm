BITS 64

start:
	mov rax, 0x8080
	push rax
	mov rbx, -1
	tzcnt bx, [rsp]
	; rbx: 0xffffffffffff0007
	; rax: 0x8080
	; flags: IF

	mov rax, -1
	mov ax, 0x8080
	mov rbx, -1
	tzcnt bx, ax
	; rbx: 0xffffffffffff0007
	; rax: 0xffffffffffff8080
	; flags: IF

	mov rax, 0x8080
	push rax
	mov rbx, -1
	tzcnt ebx, [rsp]
	; rbx: 0x7
	; rax: 0x8080
	; flags: IF

	mov rax, -1
	mov eax, 0x8080
	mov rbx, -1
	tzcnt ebx, eax
	; rbx: 0x7
	; rax: 0x8080
	; flags: IF

	mov rax, 0x8080
	push rax
	mov rbx, -1
	tzcnt rbx, [rsp]
	; rbx: 0x7
	; rax: 0x8080
	; flags: IF

	mov rax, -1
	mov eax, 0x8080
	mov rbx, -1
	tzcnt rbx, rax
	; rbx: 0x7
	; rax: 0x8080
	; flags: IF

	mov rax, -1
	tzcnt rbx, rax
	; rax: -1
	; rbx: 0
	; flags: ZF IF

	mov rax, 0
	tzcnt rbx, rax
	; rax: 0
	; rbx: 0x40
	; flags: CF IF

	db 0xcc
