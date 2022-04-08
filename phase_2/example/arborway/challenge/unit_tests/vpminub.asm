BITS 64

start:
	and rsp, 0xfffffffffffffff0
	call setit
	call setit1
	call setit2

	vpminub xmm2, xmm1, xmm0
	; ymm2: v2_int128 = {0x11223344550d770d2a2b2c2d2e0b210b, 0x0}

	call setit
	call setit2

	mov rax, 0xa0a1a2a3a4a5a6a7
	push rax
	push rax
	vpminub xmm2, xmm0, [rsp]
	add rsp, 16
	; ymm2: v2_int128 = {0xa0a1a2a3a40da60da0a1a2a3a40ba60b, 0x0}

	call setit
	call setit1
	call setit2

	vpminub ymm2, ymm1, ymm0
	; ymm2: v2_int128 = {0x11223344550d770d2a2b2c2d2e0b210b, 0x1a1b1c1d1e1f1000abacadaeafa09988}

	call setit
	call setit2

	mov rax, 0xa0a1a2a3a4a5a6a7
	push rax
	push rax
	push rax
	push rax
	vpminub ymm2, ymm0, [rsp]
	; ymm2: v2_int128 = {0xa0a1a2a3a40da60da0a1a2a3a40ba60b, 0x7766554433221100a0a1a2a3a4a59988}
	add rsp, 32
	
	db 0xcc
setit2:
	mov rax, 0x1a1b1c1d1e1f1012
	push rax
	mov rax, 0xabacadaeafa0a2a3
	push rax
	mov rax, 0x1122334455667788
	push rax
	mov rax, 0x2a2b2c2d2e2f2120
	push rax
	vmovdqu ymm2, [rsp]
	add rsp, 0x20
	ret

setit1:
	mov rax, 0x1a1b1c1d1e1f1012
	push rax
	mov rax, 0xabacadaeafa0a2a3
	push rax
	mov rax, 0x1122334455667788
	push rax
	mov rax, 0x2a2b2c2d2e2f2120
	push rax
	vmovdqu ymm1, [rsp]
	add rsp, 0x20
	ret

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


