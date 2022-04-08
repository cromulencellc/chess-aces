BITS 64

start:
	xor rcx, rcx
	xor rdx, rdx
	xor rsi, rsi
	xor rdi, rdi
	xor r11, r11
	xor r12, r12
	xor r13, r13
	mov rax, -1
	mov al, 5
	mov rbx, -1
	mov bl, 6
	mul bl
	; rax: 0xffffffffffff001e
	; flags: PF IF

	mov rax, -1
	mov al, -5
	mov rbx, -1
	mov bl, 6
	mul bl
	; rax: 0xffffffffffff05e2
	; flags: CF PF SF IF OF

	mov rax, -1
	mov al, 5
	mov rbx, -1
	mov bl, 6
	push rbx
	mul byte[rsp]
	; rax: 0xffffffffffff001e
	; flags: PF IF
	
	mov rax, -1
	mov al, -5
	mov rbx, -1
	mov bl, 6
	push rbx
	mul byte[rsp]
	; rax: 0xffffffffffff05e2
	; flags: CF PF SF IF OF

	; start 16 bit
	mov rax, -1
	mov rdx, -1
	mov ax, 5
	mov rbx, -1
	mov bx, 6
	mul bx
	; rax: 0xffffffffffff001e
	; rdx: 0xffffffffffff0000
	; flags: PF IF

	mov rax, -1
	mov rdx, -1
	mov ax, -5
	mov rbx, -1
	mov bx, 6
	mul bx
	; rax: 0xffffffffffffffe2
	; rdx: 0xffffffffffff0005
	; flags: CF PF SF IF OF

	mov rax, -1
	mov rdx, -1
	mov ax, 5
	mov rbx, -1
	mov bx, 6
	push rbx
	mul word [rsp]
	; rax: 0xffffffffffff001e
	; rdx: 0xffffffffffff0000
	; flags: PF IF
	
	mov rax, -1
	mov rdx, -1
	mov ax, -5
	mov rbx, -1
	mov bx, 6
	push rbx
	mul word [rsp]
	; rax: 0xffffffffffffffe2
	; rdx: 0xffffffffffff0005
	; flags: CF PF SF IF OF

	; start 32 bit
	mov rax, -1
	mov rdx, -1
	mov eax, 5
	mov rbx, -1
	mov ebx, 6
	mul ebx
	; rax: 0x1e
	; rdx: 0
	; flags: PF IF

	mov rax, -1
	mov rdx, -1
	mov eax, -5
	mov rbx, -1
	mov ebx, 6
	mul ebx
	; rax: 0xffffffe2
	; rdx: 0x5
	; flags: CF PF SF IF OF

	mov rax, -1
	mov rdx, -1
	mov eax, 5
	mov rbx, -1
	mov ebx, 6
	push rbx
	mul dword [rsp]
	; rax: 0x1e
	; rdx: 0x0
	; flags: PF IF

	mov rax, -1
	mov rdx, -1
	mov eax, -5
	mov rbx, -1
	mov ebx, 6
	push rbx
	mul dword [rsp]
	; rax: 0xffffffe2
	; rdx: 0x05
	; flags: CF PF SF IF OF

	; start 64 bit
	mov rdx, -1
	mov rax, 5
	mov rbx, 6
	mul rbx
	; rax: 0x1e
	; rdx: 0
	; flags: PF IF

	mov rdx, -1
	mov rax, -5
	mov rbx, 6
	mul rbx
	; rax: 0xffffffffffffffe2
	; rdx: 0x5
	; flags: CF PF SF IF OF

	mov rdx, -1
	mov rax, 5
	mov rbx, 6
	push rbx
	mul qword [rsp]
	; rax: 0x1e
	; rdx: 0x00
	; flags: PF IF

	mov rdx, -1
	mov rax, -5
	mov rbx, 6
	push rbx
	mul qword [rsp]
	; rax: 0xffffffffffffffe2
	; rdx: 0x5
	; flags: CF PF SF IF OF

	db 0xcc
