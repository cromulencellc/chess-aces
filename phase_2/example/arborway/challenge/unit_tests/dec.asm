BITS 64

start:
	mov al, 1
	dec al
	; al: 0
	; flags: PF ZF IF

	mov rax, 0xffffffff04400000
	dec eax
	; rax: 0x43fffff
	; flags: PF AF IF

	mov rax, -1
	mov al, 5
	dec al
	; rax = 0xffffffffffffff04
	; flags: IF

	mov rax, -1
	mov al, 5
	push rax
	dec byte [rsp]
	; [rsp] = 0xffffffffffffff04
	; flags: IF

	mov rax, -1
	mov al, -60
	dec al
	; rax: 0xffffffffffffffc3
	; flags: PF SF IF

	mov rax, -1
	mov al, 0
	dec al
	; rax: 0xffffffffffffffff
	; flags: PF AF SF IF

	mov rax, -1
	mov ax, 5
	dec ax
	; rax: 0xffffffffffff0004
	; flags: IF

	mov rax, -1
	mov ax, 5
	push rax
	dec word [rsp]
	; [rsp]: 0xffffffffffff0004
	; flags: IF

	mov rax, -1
	mov ax, -60
	dec ax
	; rax: 0xffffffffffffff3c
	; flags: PF SF IF

	mov rax, -1
	mov eax, 5
	dec eax
	; rax: 0x4
	; flags: IF

	mov rax, -1
	mov eax, 5
	push rax
	dec dword [rsp]
	; [rsp] 0x0000000000000004
	; flags: IF

	mov rax, -60
	dec rax
	; rax: 0xffffffffffffffc3
	; flags: PF SF IF

	mov rax, -1
	mov rax, 5
	dec rax
	; rax: 0x04
	; flags: IF

	mov rax, -1
	mov rax, 5
	push rax
	dec qword [rsp]
	; [rsp]: 0x4
	; flags: IF
	
	db 0xcc
