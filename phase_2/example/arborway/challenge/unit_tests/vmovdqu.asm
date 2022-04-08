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

	vmovdqu [rsp], ymm0
	vmovdqu ymm2, [rsp]
	vmovdqu [rsp], xmm2
	vmovdqu xmm3, [rsp]
	vmovdqu xmm4, xmm3
	vmovdqu xmm1, xmm4

	; ymm0: v2_int128 = { 0xcafebabed00dd00ddeadbeefb00bb00b, 0x7766554433221100ffeeddccbbaa9988}
	; ymm1: v2_int128 = { 0xcafebabed00dd00ddeadbeefb00bb00b, 0x0}
	; ymm2: v2_int128 = { 0xcafebabed00dd00ddeadbeefb00bb00b, 0x7766554433221100ffeeddccbbaa9988}
	; ymm3: v2_int128 = { 0xcafebabed00dd00ddeadbeefb00bb00b, 0x0}
	; ymm4: v2_int128 = { 0xcafebabed00dd00ddeadbeefb00bb00b, 0x0}







	db 0xcc
