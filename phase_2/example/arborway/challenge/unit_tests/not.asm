BITS 64

start:
	mov rax, 0xffffffff04400000
	not eax
	; rax: 0xfbbfffff

	mov rax, -1
	mov al, 5
	not al
	; rax: 0xfffffffffffffffa

	mov rax, -1
	mov al, 5
	push rax
	not byte [rsp]
	; [rsp] = 0xfffffffffffffffa

	mov rax, -1
	mov al, -60
	not al
	; rax: 0xffffffffffffff3b

	mov rax, -1
	mov al, 0
	not al
	; rax: -1

	mov rax, -1
	mov ax, 5
	not ax
	; rax: 0xfffffffffffffffa

	mov rax, -1
	mov ax, 5
	push rax
	not word [rsp]
	; rax: 0xfffffffffffffffa

	mov rax, -1
	mov ax, -60
	not ax
	; rax: 0xffffffffffff003b

	mov rax, -1
	mov eax, 5
	not eax
	; rax: 0xfffffffa

	mov rax, -1
	mov eax, 5
	push rax
	not dword [rsp]
	; [rsp] 0x00000000fffffffa

	mov rax, -60
	not rax
	; rax: 0x3b

	mov rax, -1
	mov rax, 5
	not rax
	; rax: 0xfffffffffffffffa

	mov rax, -1
	mov rax, 5
	push rax
	not qword [rsp]
	; rax: 0xfffffffffffffffa
	
	db 0xcc
