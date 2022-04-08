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

	mov rax, 0xbad5856de234b193
	movq xmm1, rax	
	; ymm1: v2_int128 = {0xbad5856de234b193, 0x7654321076543210fedcba98fedcba98}
	movq xmm0, qword [rsp] 
	; ymm0: 2_int128 = {0xb00b8008b008800b, 0x7766554433221100ffeeddccbbaa9988}

	movq [rsp], xmm0
	; (gdb) x /4gx $rsp
	; 0x7fffffffe300:	0xb00b8008b008800b	0xc0dec0dec0dec0de
	; 0x7fffffffe310:	0xfedcba98fedcba98	0x7654321076543210
	movq xmm2, qword [rsp]
	; ymm2: v2_int128 = {0xb00b8008b008800b, 0x0}

	movq xmm4, xmm2

	db 0xcc
