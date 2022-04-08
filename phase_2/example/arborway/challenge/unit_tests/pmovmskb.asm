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
	
	mov rax, -1
	pmovmskb eax, xmm0
	; rax: 0xfafa
	db 0xcc
