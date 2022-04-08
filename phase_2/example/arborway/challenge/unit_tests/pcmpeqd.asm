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
	mov rax, 0x7654321076543200
	push rax
	mov rax, 0xfedcba98fedcba88
	push rax
	mov rax, 0xcafebabed00dd00d
	push rax
	mov rax, 0xb00b8008b008800b
	push rax
	vmovdqu ymm1, [rsp]
	pcmpeqd xmm1, xmm0
	; ymm1: v2_int128 = {0xffffffffffffffff0000000000000000, 0x7654321076543200fedcba98fedcba88}

	mov rax, 0x7654321076543200
	push rax
	mov rax, 0xffeeddccbbaa9988
	push rax
	mov rax, 0xcafebabed00dd00d
	push rax
	mov rax, 0xdeadbeefb00bb00b
	push rax
	vmovdqu ymm1, [rsp]
	mov rax, 0x7654321076543200
	push rax
	mov rax, 0xfedcba98fedcba99
	push rax
	mov rax, 0xcafebabed00dd00d
	push rax
	mov rax, 0xb00b8008b008800b
	push rax
	pcmpeqd xmm1, [rsp]
	; ymm1: v2_int128 = {0xffffffffffffffff0000000000000000, 0x7654321076543200fedcba98fedcba88}
	db 0xcc
