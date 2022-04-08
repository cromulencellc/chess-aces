BITS 64

start:
	mov rax, 0xffffffff00000010
	mov rbx, 0xffffffff00000020
	cmp eax, ebx
	cmova eax, ebx
	; rax: 0x10
	; flags: CF PF SF IF

	; rax: 0x10
	; flags: CF PF SF IF
	mov rax, -1
	mov rbx, -1
	mov ax, 20
	mov bx, 10
	cmp ax, bx
	cmova ax, bx
	; rax: 0xffffffffffff000a 
	; flags: PF AF IF

	mov rax, -1
	mov rbx, -1
	mov ax, 20
	mov bx, 10
	push rbx
	cmp ax, word[rsp]
	cmova ax, word[rsp]
	; rax: 0xffffffffffff000a 
	; flags: PF AF IF

	mov rax, -1
	mov rbx, -1
	mov ax, 10
	mov bx, 20
	cmp ax, bx
	cmova ax, bx
	; rax: 0xffffffffffff000a 
	; flags: CF PF SF IF

	mov rax, -1
	mov rbx, -1
	mov ax, 10
	mov bx, 20
	push rbx
	cmp ax, word[rsp]
	cmova ax, word[rsp]
	; rax: 0xffffffffffff000a 
	; flags: CF PF SF IF

	mov rax, 0xffffffff00000020
	mov rbx, 0xffffffff00000010
	cmp eax, ebx
	cmova eax, ebx
	; rax: 0x10
	; flags: IF

	mov rax, 0xffffffff00000020
	mov rbx, 0xffffffff00000010
	push rbx
	cmp eax, dword[rsp]
	cmova eax, dword [rsp]
	; rax: 0x10
	; flags: IF

	mov rax, 0xffffffff00000010
	mov rbx, 0xffffffff00000020
	cmp eax, ebx
	cmova eax, ebx
	; rax: 0x10
	; flags: CF PF SF IF

	mov rax, 0xffffffff00000010
	mov rbx, 0xffffffff00000020
	push rbx
	cmp eax, dword [rsp]
	cmova eax, dword [rsp]
	; rax: 0x10
	; flags: CF PF SF IF

	mov rax, 0xffffffff00000020
	mov rbx, 0xffffffff00000010
	cmp rax, rbx
	cmova rax, rbx
	; rax: 0xffffffff00000010
	; flags: IF
	
	mov rax, 0xffffffff00000020
	mov rbx, 0xffffffff00000010
	push rbx
	cmp rax, qword[rsp]
	cmova rax, qword [rsp]
	; rax: 0xffffffff00000010
	; flags: IF

	mov rax, 0xffffffff00000010
	mov rbx, 0xffffffff00000020
	cmp rax, rbx
	cmova rax, rbx
	; rax: 0xffffffff00000010
	; flags: CF PF SF IF

	mov rax, 0xffffffff00000010
	mov rbx, 0xffffffff00000020
	push rbx
	cmp rax, qword [rsp]
	cmova rax, qword [rsp]
	; rax: 0xffffffff00000010
	; flags: CF PF SF IF
	db 0xcc
