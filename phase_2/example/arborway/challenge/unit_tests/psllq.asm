BITS 64

start:
	mov rax, 0x7654321076543200
	push rax
	mov rax, 0xffeeddccbbaa9988
	push rax
	mov rax, 0xcafebabed00dd00d
	push rax
	mov rax, 0xdeadbeefb00bb00b
	push rax
	vmovdqu ymm1, [rsp]
	psllq xmm1, 32
	; ymm1: v2_int128 = {0xd00dd00d00000000b00bb00b00000000, 0x7654321076543200ffeeddccbbaa9988}

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
	psllq xmm0, 7
	; ymm0: v2_int128 = {0x7f5d5f6806e8068056df77d805d80580, 0x7654321076543200ffeeddccbbaa9988}


	mov rax, 0x7654321076543200
	push rax
	mov rax, 0xffeeddccbbaa9988
	push rax
	mov rax, 0xcafebabed00dd00d
	push rax
	mov rax, 0xdeadbeefb00bb00b
	push rax
	vmovdqu ymm1, [rsp]
	xor rax, rax
	push rax
	push rax
	push rax
	mov rax, 7
	push rax
	psllq xmm1, [rsp]
	; ymm1: v2_int128 = {0x7f5d5f6806e8068056df77d805d80580, 0x7654321076543200ffeeddccbbaa9988}
	
	mov rax, 0x7654321076543200
	push rax
	mov rax, 0xffeeddccbbaa9988
	push rax
	mov rax, 0xcafebabed00dd00d
	push rax
	mov rax, 0xdeadbeefb00bb00b
	push rax
	vmovdqu ymm1, [rsp]
	psllq xmm1, 16
	; ymm1: v2_int128 = { 0xbabed00dd00d0000beefb00bb00b0000, 0x7654321076543200ffeeddccbbaa9988}

	db 0xcc
