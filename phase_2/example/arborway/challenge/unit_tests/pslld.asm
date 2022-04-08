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
	pslld xmm1, 32
	; ymm1: v2_int128 = {0x0, 0x7654321076543200ffeeddccbbaa9988}


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
	pslld xmm0, 7
	; ymm0: v2_int128 = {0x7f5d5f0006e8068056df778005d80580, 0x7654321076543200ffeeddccbbaa9988}

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
	pslld xmm1, [rsp]
	; ymm0: v2_int128 = {0x7f5d5f0006e8068056df778005d80580, 0x7654321076543200ffeeddccbbaa9988}
	
	mov rax, 0x7654321076543200
	push rax
	mov rax, 0xffeeddccbbaa9988
	push rax
	mov rax, 0xcafebabed00dd00d
	push rax
	mov rax, 0xdeadbeefb00bb00b
	push rax
	vmovdqu ymm1, [rsp]
	pslld xmm1, 16
	; ymm1: v2_int128 = { 0xbabe0000d00d0000beef0000b00b0000, 0x7654321076543200ffeeddccbbaa9988}


	db 0xcc
