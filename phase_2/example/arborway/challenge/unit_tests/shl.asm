BITS 64

start:
	mov rax, 0xaa
	shl al, 8
	; rax = 0
	; flags: PF ZF IF OF
	mov rax, -1
	shl al, 1
	; rax = 0xfffffffffffffffe
	; flags: CF SF IF
	mov rax, 0xffffffffffffffff
	shl eax, 32	
	; rax = 0xffffffff
	; flags: CF SF IF ## technically they are unaffected and the same
	mov rax, 0xffffffffffffff2f
	shl al, 1
	; rax = 0xffffffffffffff5e
	; flags: IF
	mov rax, 0xffffffffffffff2f
	shl ax, 1
	; rax = 0xfffffffffffffe5e
	; flags: CF SF IF
	mov rax, 0xffffffffffffff2f
	shl eax, 1
	; rax = 0xfffffe5e
	; flags: CF SF IF
	mov rax, 0xffffffffffffffff
	shl eax, 33
	; rax = 0xfffffffe
	; flags: CF SF IF
	mov rax, 0xffffffff11223344
	shl rax, 3
	; rax: 0xfffffff889119a20
	; flags: CF SF IF
	mov rax, 0x93
	shl rax, 3
	; rax: 0x498
	; flags: IF
	db 0xcc
