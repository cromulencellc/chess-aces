BITS 64

start:
	mov rax, 0x4080
	mov rbx, 0x407f
	cmp al, bl
	; rax = 0x4001
	; flags: AF IF OF 
	mov rax, -1
	cmp rax, -1
	; rax = 0
	; flags: PF ZF IF
	mov rax, -1
	cmp ax, 5
	; rax = 0xfffffffffffffffa
	; flags: PF SF IF
	mov rbx, 10
	mov rcx, 10
	push rcx
	cmp rbx, [rsp]
	pop rcx
	; rbx = 0
	; flags: PF ZF IF
	mov rax, 8
	cmp rax, rcx
	; rax = 0xfffffffffffffffe
	; flags: CF AF SF IF
	db 0xcc
