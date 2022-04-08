BITS 64

start:
	mov rdx, 0xf999999999999999
	mov rax, 1
	mov rbx, 5
	idiv rbx
	mov rax, 0xe7
	syscall
