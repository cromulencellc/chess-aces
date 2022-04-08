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

	paddd xmm0, xmm1
	; ymm0: v2_int128 = {0x8bdd7b9c90ec90eb8eb93ef760143016, 0x7766554433221100ffeeddccbbaa9988}


	paddd xmm0, [rsp]
	; ymm0: v2_int128 = {0x4cbc3c7a51cb51c93ec4beff101cb021, 0x7766554433221100ffeeddccbbaa9988}


	db 0xcc
