BITS 64

start:
	and rsp, 0xfffffffffffffff0

	call setit
	call setit1
	
	vpbroadcastb xmm1, xmm0
	; ymm1: v2_int128 = {0xb0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b, 0x0}

	call setit1
	vpbroadcastb ymm0, xmm1
	; ymm0: v2_int128 = {0x20202020202020202020202020202020, 0x20202020202020202020202020202020}
	db 0xcc

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


