BITS 64

start:
	and rsp, 0xfffffffffffffff0
	call setit

	mov eax, 0x35cd37ef
	movd xmm0, eax
	; ymm0: v2_int128 = {0x35cd37ef, 0x7766554433221100ffeeddccbbaa9988}

	call setit

	mov eax, 0x35cd37ef
	push rax
	movd xmm0, dword [rsp]
	; ymm0: v2_int128 = {0x35cd37ef, 0x7766554433221100ffeeddccbbaa9988}
	pop rax

	call setit

	mov rax, -1
	movd eax, xmm0
	; rax: 0xb00bb00b
	movd dword [rsp], xmm0
	; [rsp] 0x00000000b00bb00b

	db 0xcc

setit:
	mov rax, 0x7766554433221100
	push rax
	mov rax, 0xffeeddccbbaa9988
	push rax
	mov rax, 0xcafebabed00dd00d
	push rax
	mov rax, 0xdeadbeefb00bb00b
	push rax
	vmovdqu ymm0, [rsp]
	add rsp, 0x20
	ret


