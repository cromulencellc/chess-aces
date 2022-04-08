BITS 64

start:
	xor rax, rax
	mov rax, -1
	mov al, 0x80

	mov rcx, -1
	mov rbx, -1
	mov rdx, -1

	movzx cx, al
	movzx ebx, al
	movzx rdx, al

	xor rax, rax
	mov rax, -1
	mov ax, 0x8000

	mov rcx, -1
	mov rbx, -1

	movzx ecx, ax
	movzx rbx, ax
	
	db 0xcc
