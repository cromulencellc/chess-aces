BITS 64

start:
	xor rdx, rdx
	xor rcx, rcx
	xor rbx, rbx

	mov rax, 0xdeadbeefcafebabe
	mov rdx, 1
	mov rbx, -5
	idiv rbx
	; rax: 0xa043a69cd766a774
	; rdx: 2
	
	mov rax, -1
	mov eax, 0x12345678
	mov rdx, -1
	mov ebx, 5
	idiv ebx
	; rax: 0xd070de18
	; rdx: 0

	mov rax, -1
	mov ax, 0x20
	mov dx, -1
	mov bx, -5
	idiv bl
	; rax: 0xffffffffffff02fa
	; rdx: 0xffff

	push -5
	mov ax, 0x20
	mov dx, 1
	idiv byte [rsp]
	pop bx
	; rax: 0xffffffffffff02fa
	; rdx: 1

	mov rax, 0xffffffffffffffff
	mov rdx, 0xffffffffffffffff
	mov rbx, 2
	idiv rbx	
	; rax: 0
	; rdx: -1

	db 0xcc
