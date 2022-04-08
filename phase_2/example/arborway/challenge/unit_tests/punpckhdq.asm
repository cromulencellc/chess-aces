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

	mov rax, 0xec7bae8ca9eec6e6
	push rax
	mov rax, 0xac9087be87564f87
	push rax
	mov rax, 0x8343427895dbeeee
	push rax
	mov rax, 0xdeadbeef87dfb00b
	push rax
	vmovdqu ymm1, [rsp]

	punpckhdq xmm0, xmm1
	; ymm0: v2_int128 = {0x83434278cafebabe95dbeeeed00dd00d, 0x7654321076543200ffeeddccbbaa9988}


 
	mov rax, 0x7654321076543200
	push rax
	mov rax, 0xffeeddccbbaa9988
	push rax
	mov rax, 0xcafebabed00dd00d
	push rax
	mov rax, 0xdeadbeefb00bb00b
	push rax
	vmovdqu ymm0, [rsp]

	mov rax, 0xec7bae8ca9eec6e6
	push rax
	mov rax, 0xac9087be87564f87
	push rax
	mov rax, 0x8343427895dbeeee
	push rax
	mov rax, 0xdeadbeef87dfb00b
	push rax

	punpckhdq xmm0, [rsp]
	; ymm0: v2_int128 = {0x83434278cafebabe95dbeeeed00dd00d, 0x7654321076543200ffeeddccbbaa9988}

	db 0xcc
