BITS 64

start:
	mov rdi, -1
	mov rcx, -1
	mov rbx, -1

	mov eax, 0x80000000
	push rax
	movsxd rcx, dword [rsp]
	movsxd rbx, eax

	xor rdi, rdi
	xor rcx, rcx
	xor rbx, rbx

	mov eax, 0x7fffffff

	movsxd rcx, eax
	movsxd rbx, eax

	db 0xcc
