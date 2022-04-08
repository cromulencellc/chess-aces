BITS 64

start:
	mov rax, -1
	mov eax, 0xdeadbeef

	mov rax, -1
	mov ax, 0xbeef

	mov rax, -1
	mov al, 0xef

	mov rax, -1
	mov ah, 0xbe

	mov rax, -1
	mov rcx, rax
	mov rdi, rax
	mov rbx, rax
	mov rsi, rax
	mov rax, 0xdeadbeefcafebabe
	push rax
	mov rbx, [rsp]
	mov edi, [rsp]
	mov si, [rsp]
	mov cl, [rsp]
	mov ch, [rsp]

	pop rax
	mov rax, 0xb00bb00b80088008
	mov rbx, -1
	push rbx
	mov byte[rsp], al
	pop rbx
	mov rbx, -1
	push rbx
	mov word [rsp], ax
	pop rbx
	mov rbx, -1
	push rbx
	mov dword [rsp], eax

	pop rbx
	mov rbx, -1
	push rbx
	mov qword [rsp], rax
	db 0xcc
