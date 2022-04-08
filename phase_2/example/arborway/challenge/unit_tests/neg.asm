BITS 64

start:
	mov rax, 0xffffffff04400000
	neg eax
	; rax: 0xfbc00000
	; flags: CF PF SF IF

	mov rax, -1
	mov al, 5
	neg al
	; rax = 0xfffffffffffffffb
	; flags: CF AF SF IF

	mov rax, -1
	mov al, 5
	push rax
	neg byte [rsp]
	; [rsp] = 0xfffffffffffffffb
	; flags: CF AF SF IF

	mov rax, -1
	mov al, -60
	neg al
	; rax: 0xffffffffffffff3c
	; flags: CF PF AF IF

	mov rax, -1
	mov al, 0
	neg al
	; rax: 0xffffffffffffff00
	; flags: PF ZF IF

	mov rax, -1
	mov ax, 5
	neg ax
	; rax: 0xfffffffffffffffb
	; flags: CF AF SF IF

	mov rax, -1
	mov ax, 5
	push rax
	neg word [rsp]
	; rax: 0xfffffffffffffffb
	; flags: CF AF SF IF

	mov rax, -1
	mov ax, -60
	neg ax
	; rax: 0xffffffffffff003c
	; flags: CF PF AF IF

	mov rax, -1
	mov eax, 5
	neg eax
	; rax: 0xfffffffb
	; flags: CF AF SF IF

	mov rax, -1
	mov eax, 5
	push rax
	neg dword [rsp]
	; [rsp] 0x00000000fffffffb
	; flags: CF AF SF IF

	mov rax, -60
	neg rax
	; rax: 0x3c
	; flags: CF PF AF IF

	mov rax, -1
	mov rax, 5
	neg rax
	; rax: 0xfffffffffffffffb
	; flags: CF AF SF IF

	mov rax, -1
	mov rax, 5
	push rax
	neg qword [rsp]
	; rax: 0xfffffffffffffffb
	; flags: CF AF SF IF
	
	db 0xcc
