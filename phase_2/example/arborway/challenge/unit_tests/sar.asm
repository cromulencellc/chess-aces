BITS 64

start:
	mov rbx, 0x80
	sar bl, 1
	; rbx = 0xc0
	; flags: PF SF IF
	mov rbx, 7
	sar bl, 1
	; rbx = 3
	; flags: CF PF IF
	mov rbx, -1
	mov rcx, 10
	sar rbx, cl
	; rbx = 0xffffffffffffffff
	; rcx = 0xa
	; flags: CF PF SF IF
	mov rbx, -1
	sar ebx, 20
	; rbx = 0xffffffff
	; flags: CF PF SF IF
	mov rbx, -1
	sar bx, 10
	; rbx = 0xffffffffffffffff
	; flags: CF PF SF IF
	mov rbx, -1
	sar bl, 8
	; rbx = 0xffffffffffffffff
	; flags: CF PF SF IF
	mov rbx, -1
	push rbx
	sar dword [rsp], 8
	db 0xcc
