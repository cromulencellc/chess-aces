BITS 64

start:
	and rsp, 0xfffffffffffffff0
	mov rax, 0x7766554433221100
	push rax
	mov rax, 0xffeeddccbbaa9988
	push rax
	mov rax, 0xcafebabed00dd00d
	push rax
	mov rax, 0xdeadbeefb00bb00b
	push rax
	vmovdqu ymm0, [rsp]
	mov rax, 0x7654321076543210
	push rax
	mov rax, 0xfedcba98fedcba98
	push rax
	mov rax, 0xc0dec0dec0dec0de
	push rax
	mov rax, 0xb00b8008b008800b
	push rax
	vmovdqu ymm1, [rsp]
	pxor xmm1, xmm0
	; ymm1: v2_int128 = {0xa207a6010d310d36ea63ee700033000, 0x7654321076543210fedcba98fedcba98}
	; ymm0: v2_int128 = {0xcafebabed00dd00ddeadbeefb00bb00b, 0x7766554433221100ffeeddccbbaa9988}

	mov rax, 0x7766554433221100
	push rax
	mov rax, 0xffeeddccbbaa9988
	push rax
	mov rax, 0xcafebabed00dd00d
	push rax
	mov rax, 0xdeadbeefb00bb00b
	push rax
	vmovdqu ymm1, [rsp]
	mov rax, 0x7654321076543210
	push rax
	mov rax, 0xfedcba98fedcba98
	push rax
	mov rax, 0xc0dec0dec0dec0de
	push rax
	mov rax, 0xb00b8008b008800b
	push rax
	vmovdqu ymm0, [rsp]
	pxor xmm1, [rsp]

	; ymm1: v2_int128 = {0xa207a6010d310d36ea63ee700033000, 0x7654321076543210fedcba98fedcba98}
	db 0xcc
