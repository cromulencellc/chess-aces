BITS 64

start:
	; flag tests
	xor rax, rax
	; rax = 0
	; flags: PF ZF IF
	mov rax, 0xdeadbeefcafebabe
	mov rbx, 0
	xor rbx, rax
	; rbx = rax
	; flags: PF SF IF

	; test the zeroing of the upper bits
	mov rax, 0xdeadbeefcafebabe
	mov rbx, 0x1337d00db00bd00d
	xor eax, ebx
	; The following section just tests xoring memory locations
	xor rax, rax
	sub rsp, 0x60
	mov rdi, rsp
	mov rcx, 12
	rep stosq
	; clear some space

	mov rbx, 0xdeadbeefcafebabe
	xor qword [rsp], rbx
	pop rcx

	xor dword [rsp], ebx
	pop rcx

	xor word [rsp], bx
	pop rcx

	xor byte [rsp], bl
	pop rcx	

	db 0xcc
