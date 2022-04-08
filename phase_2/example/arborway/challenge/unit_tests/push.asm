BITS 64

start:
	mov rax, 0xdeadbeefcafebabe
	push ax
	push rax
	mov rbx, rsp
	push word [rsp]
	push qword [rsp]
	push 1
	push 0xdeadbee
	db 0xcc
