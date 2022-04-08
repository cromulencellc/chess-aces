BITS 64

start:
	xor rcx, rcx
	xor rdx, rdx
	xor rsi, rsi

	mov rcx, -1
	xor rbx, rbx
	xor rbx, rbx
	imul rcx, rbx
	; used to test ZF

	mov rcx, -1
	mov rcx, 0x8000000000000000
	mov rbx, 0x3
	imul rcx, rbx

	mov rcx, -1
	mov rcx, 0x54348fffffffffff
	mov rbx, 0x84
	imul rcx, rbx

	mov rcx, 0x54348fffffffffff
	mov rbx, 0x84
	push rbx
	imul rcx, qword [rsp]
	pop rbx

	mov rcx, -1
	mov ecx, 0x54348fff
	mov ebx, 0x84
	imul ecx, ebx

	mov rcx, -1
	mov ecx, 0x54348fff
	mov ebx, 0x84
	push rbx
	imul ecx, dword [rsp]
	pop rbx

	mov rcx, -1
	mov cx, 0x5434
	mov bx, 0x84
	imul cx, bx

	mov rcx, -1
	mov cx, 0x5434
	mov rbx, 0x84
	push rbx
	imul cx, word [rsp]
	pop rbx
	
	db 0xcc
