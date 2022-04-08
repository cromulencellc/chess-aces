BITS 64

start:
	and rsp, 0xfffffffffffffff0
	call setit

	mov eax, 0x35cd37ef
	vmovd xmm0, eax
	; ymm0: v2_int128 = {0x35cd37ef, 0x0}

	call setit

	mov eax, 0x35cd37ef
	push rax
	vmovd xmm0, dword [rsp]
	; ymm0: v2_int128 = {0x35cd37ef, 0x0}
	pop rax

	call setit

	mov rax, -1
	vmovd eax, xmm0
	; rax: 0xb00bb00b
	vmovd dword [rsp], xmm0
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


