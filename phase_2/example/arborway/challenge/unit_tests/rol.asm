BITS 64

start:	
	mov rax, 0xdeadbeefcafebabe
	rol eax, 0
	; rax: 0xcafebabe
	; flags: PF IF

	mov rax, 0x8000000000000000
	rol rax, 1
	; rax: 1
	; flags: CF PF IF OF

	mov rax, 0xc000000000000000
	rol rax, 1
	; rax: 0x8000000000000001
	; flags: CF PF IF

	mov rax, 0
	rol rax, 1
	; PF IF

	; rol r/m8, imm
	mov r8, 0x1122334455667788
	rol r8b, 18
	; r8: 0x1122334455667722
	; flags: PF IF

	mov r8, 0x1122334455667788
	push r8
	rol byte[rsp], 18
	; [rsp]: 0x1122334455667722
	; flags: PF IF

	; rol r/m8, cl
	mov r8, 0x1122334455667788
	mov cl, 2
	rol r8b, cl
	; r8: 0x1122334455667722
	; flags: PF IF OF

	mov r8, 0x1122334455667788
	push r8
	mov cl, 2 
	rol byte[rsp], cl
	; [rsp]: 0x1122334455667722
	; flags: PF IF OF
	
	; rol r/m16, imm
	mov r8, 0x1122334455667788
	rol r8w, 23
	; r8: 0x112233445566c43b
	; flags: CF PF IF OF

	mov r8, 0x1122334455667788
	push r8
	rol word [rsp], 23
	; [rsp[: 0x112233445566c43b
	; flags: CF PF IF OF

	; rol r/m16, cl
	mov r8, 0x1122334455667788
	mov cl, 8
	rol r8w, cl
	; r8: 0x1122334455668877
	; flags: CF PF IF OF

	mov r8, 0x1122334455667788
	push r8
	mov cl, 8
	rol word [rsp], cl
	; [rsp]: 0x1122334455668877
	; flags: CF PF IF OF
	
	; rol r/m32, imm
	mov r8, 0x1122334455667788
	rol r8d, 37
	; r8: 0xaccef10a
	; flags: PF IF OF

	mov r8, 0x1122334455667788
	push r8
	rol dword [rsp], 37
	; [rsp]: 0x11223344accef10a
	; flags: PF IF OF

	; rol r/m32, cl
	mov r8, 0x1122334455667788
	mov cl, 15
	rol r8d, cl
	; r8: 0x3bc42ab3
	; flags: CF PF IF OF

	mov r8, 0x1122334455667788
	push r8
	mov cl, 15
	rol dword [rsp], cl
	; [rsp] 0x112233443bc42ab3
	; flags: CF PF IF OF

	; rol r/m64, imm
	mov r8, 0x1122334455667788
	rol r8, 49
	; r8: 0xef1022446688aacc
	; flags: PF IF OF

	mov r8, 0x1122334455667788
	push r8
	rol qword [rsp], 49
	; [rsp]: 0xef1022446688aacc
	; flags: PF IF
	; rol r/m64, cl

	mov r8, 0x1122334455667788
	mov cl, 15
	rol r8, cl
	; r8: 0x19a22ab33bc40891
	; flags: CF PF IF

	mov r8, 0x1122334455667788
	push r8
	mov cl, 15
	rol qword [rsp], cl
	; [rsp]: 0x19a22ab33bc40891
	; flags: CF PF IF

	db 0xcc
