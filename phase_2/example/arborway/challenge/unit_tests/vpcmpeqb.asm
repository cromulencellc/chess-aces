BITS 64

start:
	mov rax, 0x7766554433221100
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
	mov rax, 0xc0dec0dec0dec00d
	push rax
	mov rax, 0xb00b8008b008800b
	push rax
	vmovdqu ymm1, [rsp]
	mov rax, 0xde978fe98d74d3f5
	push rax
	mov rax, 0xad8467cfe789cbc1
	push rax
	mov rax, 0xcb56bca709678b9e
	push rax
	mov rax, 0xca54bcabf5567123
	push rax
	vmovdqu ymm2, [rsp]
	vpcmpeqb ymm2, ymm1, ymm0
	; ymm2: v2_int128 = {0xff00000000ff0000ff, 0xff00000000000000ff}

	mov rax, 0x7766554433221100
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
	mov rax, 0xfedcba98fedcba99
	push rax
	mov rax, 0xc0dec0dec0dec00d
	push rax
	mov rax, 0xb00b8008b008800b
	push rax
	vmovdqu ymm1, [rsp]
	mov rax, 0xde978fe98d74d3f5
	push rax
	mov rax, 0xad8467cfe789cbc1
	push rax
	mov rax, 0xcb56bca709678b9e
	push rax
	mov rax, 0xcaadbcabf556710b
	push rax
	vmovdqu ymm2, [rsp]
	vpcmpeqb xmm2, xmm1, xmm0

	; ymm2: v2_int128 = {0xff00000000ff0000ff, 0x0}
	db 0xcc
