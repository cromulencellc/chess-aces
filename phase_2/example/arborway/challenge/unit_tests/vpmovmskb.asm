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
	
	mov rax, -1
	vpmovmskb eax, xmm0
	; rax: 0xfafa

	mov rax, -1
	vpmovmskb eax, ymm0
	; rax: 0xfffafa

	mov rax, -1
	vpmovmskb rax, xmm1
	; rax: 0xfeaa

	mov rax, -1
	vpmovmskb rax, ymm1
	; rax: 0xfffeaa
	db 0xcc
