BITS 64

start:
	mov rax, -1
	mov rbx, 0x20
	movsx eax, bl
	; rax: 0x20
	
	mov rax, -1
	movsx ax, bl
	; rax: 0xffffffffffff0020

	xor rax, rax
	mov al, 0xff
	movsx ax, al
	; rax: 0xffff

	xor rax, rax
	mov al, 0xff
	push rax
	movsx ax, byte [rsp]
	; rax: 0xffff

	xor rax, rax
	mov al, 0xff
	movsx eax, al
	; rax: 0xffffffff

	xor rax, rax
	mov al, 0xff
	push rax
	movsx eax, byte [rsp]
	; rax: 0xffffffff

	xor rax, rax
	mov al, 0xff
	movsx rax, al
	; rax: 0xffffffffffffffff

	xor rax, rax
	mov al, 0xff
	push rax
	movsx rax, byte [rsp]
	; rax: 0xffffffffffffffff

	; start 16 bit
	xor rax, rax
	mov ax, 0xffff
	movsx eax, ax
	; rax: 0xffffffff

	xor rax, rax
	mov ax, 0xffff
	push rax
	movsx eax,word [rsp]
	; rax: 0xffffffff

	xor rax, rax
	mov ax, 0xffff
	movsx rax, ax
	; rax: 0xffffffffffffffff

	xor rax, rax
	mov ax, 0xff
	push rax
	movsx rax, word [rsp]
	; rax: 0xffffffffffffffff

	db 0xcc
