BITS 64

start:
	and rsp, 0xfffffffffffffff0
	mov rax, 0x7654321076543200
	push rax
	mov rax, 0xffeeddccbbaa9988
	push rax
	mov rax, 0xcafebabed00dd00d
	push rax
	mov rax, 0xdeadbeefb00bb00b
	push rax
	vmovdqu ymm0, [rsp]
	pslldq xmm0, 7
	; ymm0: v2_int128 = {0xddeadbeefb00bb00b00000000000000, 0x7654321076543200ffeeddccbbaa9988}

	mov rax, 0x7654321076543200
	push rax
	mov rax, 0xffeeddccbbaa9988
	push rax
	mov rax, 0xcafebabed00dd00d
	push rax
	mov rax, 0xdeadbeefb00bb00b
	push rax
	vmovdqu ymm1, [rsp]
	pslldq xmm1, 16
	; ymm1: v2_int128 = {0x0, 0x7654321076543200ffeeddccbbaa9988}

	db 0xcc
