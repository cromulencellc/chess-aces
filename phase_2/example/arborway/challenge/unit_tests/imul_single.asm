BITS 64

start:
	xor rcx, rcx
	xor rdx, rdx
	xor rsi, rsi

	mov rax, -1
	xor rbx, rbx
	xor rbx, rbx
	imul rbx
	; used to test ZF

	mov rax, -1
	mov rax, 0x8000000000000000
	mov rbx, 0x3
	imul rbx

	mov rax, -1
	mov rax, 0x54348fffffffffff
	mov rbx, 0x84
	imul rbx

	mov rax, 0x54348fffffffffff
	mov rbx, 0x84
	push rbx
	imul qword [rsp]
	pop rbx

	mov rax, -1
	mov eax, 0x54348fff
	mov ebx, 0x84
	imul ebx

	mov rax, -1
	mov eax, 0x54348fff
	mov ebx, 0x84
	push rbx
	imul dword [rsp]
	pop rbx

	mov rax, -1
	mov ax, 0x5434
	mov bx, 0x84
	imul bx

	mov rax, -1
	mov ax, 0x5434
	mov rbx, 0x84
	push rbx
	imul word [rsp]
	pop rbx
	
	mov rax, -1
	mov al, 0x80
	mov bl, 0x5
	imul bl

	mov rax, -1
	mov al, 0x5
	mov rbx, 0x5
	push rbx
	imul byte[rsp]
	pop rbx

	db 0xcc
