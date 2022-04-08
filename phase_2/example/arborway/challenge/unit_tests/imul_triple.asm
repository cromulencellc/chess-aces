BITS 64

start:
	xor rcx, rcx
	xor rdx, rdx
	xor rsi, rsi

	xor rcx, rcx
	mov rcx, -1
	mov rbx, 1
	imul rcx, rbx, 0
	; used to test ZF

	mov rcx, 0x8000000000000000
	imul rbx, rcx, 3

	mov rcx, 0x54348fffffffffff
	mov rbx, 0x84
	imul rbx, rcx, 0x84

	mov rcx, 0x54348fffffffffff
	push rcx
	imul rbx, qword [rsp], 0x84
	pop rcx

	mov rcx, -1
	mov ecx, 0x54348fff
	imul ebx, ecx, 0x84

	mov rcx, -1
	mov ecx, 0x54348fff
	push rcx
	imul ebx, dword [rsp], 0x84
	pop rcx

	mov rcx, -1
	mov cx, 0x5434
	imul bx, cx, 0x84

	mov rcx, -1
	mov cx, 0x5434
	push rcx
	imul bx, word [rsp], 0x84
	pop rcx
	
	db 0xcc
