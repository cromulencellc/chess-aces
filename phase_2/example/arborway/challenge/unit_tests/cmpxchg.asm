BITS 64

start:
	mov rax, -1
	mov rbx, -1
	mov rcx, -1
	mov al, 5
	mov cl, 6
	mov bl, 7	
	cmpxchg cl, bl
	;  rax = 0xffffffffffffff06
	;  rbx = 0xffffffffffffff07
	;  rcx = 0xffffffffffffff06
	; flags: CF PF AF SF IF

	mov rax, -1
	mov rbx, -1
	mov al, 6
	push rax
	mov al, 5
	mov bl, 7	
	cmpxchg byte [rsp], bl
	;  rax = 0xffffffffffffff06
	;  rbx = 0xffffffffffffff07
	;  [rsp] = 0xffffffffffffff06
	; flags: CF PF AF SF IF

	mov rax, -1
	mov rbx, -1
	mov rcx, -1
	mov al, 5
	mov cl, 5
	mov bl, 7	
	cmpxchg cl, bl
	; rax = 0xffffffffffffff05
	; rbx = 0xffffffffffffff07
	; rcx = 0xffffffffffffff07
	; flags: PF ZF IF

	mov rax, -1
	mov rbx, -1
	mov al, 5
	push rax
	mov al, 5
	mov bl, 7	
	cmpxchg byte [rsp], bl
	; rax = 0xffffffffffffff05
	; rbx = 0xffffffffffffff07
	; [rsp] = 0xffffffffffffff07
	; flags: PF ZF IF

	mov rax, -1
	mov rbx, -1
	mov rcx, -1
	mov ax, 5
	mov cx, 6
	mov bx, 7	
	cmpxchg cx, bx
	; rax = 0xffffffffffff0006
	; rbx = 0xffffffffffff0007
	; rcx = 0xffffffffffff0006
	; flags: CF PF AF SF IF

	mov rax, -1
	mov rbx, -1
	mov ax, 6
	push rax
	mov ax, 5
	mov bx, 7	
	cmpxchg word [rsp], bx
	; rax = 0xffffffffffff0006
	; rbx = 0xffffffffffff0007
	; [rsp] = 0xffffffffffff0006
	; flags: CF PF AF SF IF
	
	mov rax, -1
	mov rbx, -1
	mov rcx, -1
	mov ax, 5
	mov cx, 5
	mov bx, 7	
	cmpxchg cx, bx
	; rax = 0xffffffffffff0005
	; rbx = 0xffffffffffff0007
	; rcx = 0xffffffffffff0007
	; flags: PF ZF IF

	mov rax, -1
	mov rbx, -1
	mov ax, 5
	push rax
	mov ax, 5
	mov bx, 7	
	cmpxchg word [rsp], bx
	; rax = 0xffffffffffff0005
	; rbx = 0xffffffffffff0007
	; [rsp] = 0xffffffffffff0007
	; flags: PF ZF IF

	mov rax, -1
	mov rbx, -1
	mov rcx, -1
	mov eax, 5
	mov ecx, 6
	mov ebx, 7	
	cmpxchg ecx, ebx
	; rax = 6
	; rbx = 7
	; rcx = 6
	; flags: CF PF AF SF IF

	mov rax, -1
	mov rbx, -1
	mov eax, 6
	push rax
	mov eax, 5
	mov ebx, 7	
	cmpxchg dword [rsp], ebx
	; rax = 6
	; rbx = 7
	; [rsp] = 6
	; flags: CF PF AF SF IF

	mov rax, -1
	mov rbx, -1
	mov rcx, -1
	mov eax, 5
	mov ecx, 5
	mov ebx, 7	
	cmpxchg ecx, ebx
	; rax = 5
	; rbx = 7
	; rcx = 7
	; flags: PF ZF IF

	mov rax, -1
	mov rbx, -1
	mov eax, 5
	push rax
	mov eax, 5
	mov ebx, 7	
	cmpxchg dword [rsp], ebx
	; rax = 5
	; rbx = 7
	; [rsp] = 7
	; flags: PF ZF IF

	mov rax, 5
	mov rcx, 6
	mov rbx, 7	
	cmpxchg rcx, rbx
	; rax = 6
	; rbx = 7
	; rcx = 6
	; flags: CF PF AF SF IF

	mov rax, 6
	push rax
	mov rax, 5
	mov rbx, 7	
	cmpxchg qword [rsp], rbx
	; rax = 6
	; rbx = 7
	; rcx = 6
	; flags: CF PF AF SF IF

	mov rax, 5
	mov rcx, 5
	mov rbx, 7	
	cmpxchg rcx, rbx
	; rax = 5
	; rbx: 7
	; rcx: 7
	; flags: PF ZF IF

	mov rax, 5
	push rax
	mov rax, 5
	mov rbx, 7	
	cmpxchg qword [rsp], rbx
	; rax = 5
	; rbx: 7
	; [rsp]: 7
	; flags: PF ZF IF
	
	db 0xcc
