BITS 64

start:
	xor rdx, rdx
	xor rcx, rcx
	xor rbx, rbx

	mov rax, 0xdeadbeefcafebabe
	mov rdx, 1
	mov rbx, 5
	div rbx
	
	mov rax, -1
	mov eax, 0x12345678
	mov rdx, 1
	mov ebx, 5
	div ebx


	mov rax, -1
	mov ax, 0x20
	mov dx, 1
	mov bx, 5
	div bl

	push 5
	mov ax, 0x20
	mov dx, 1
	div byte [rsp]
	pop bx

	mov rax, 0xffffffffffffffff
	mov rdx, 0xffffffffffffffff
	mov rbx, 2
	div rbx
	db 0xcc
