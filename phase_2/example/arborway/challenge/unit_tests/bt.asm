BITS 64

start:
	mov r13, 0x2
	mov rcx, 0
	bt r13, rcx
	; PF CF = 0
	add rcx, 1
	bt r13, rcx
	; CF = 1
	add rcx, 1
	bt r13, rcx
	; CF = 0
	add rcx, 1
	bt r13, rcx
	; CF = 0
	db 0xcc
