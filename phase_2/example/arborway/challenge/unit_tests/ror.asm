BITS 64

start:	
	mov rax, 0xdeadbeefcafebabe
	ror eax, 0
	; rax: 0xcafebabe
	; flags: PF IF

	mov rax, 0x0000000000000001
	ror rax, 1
	; rax: 0x8000000000000000
	; flags: CF PF IF OF

	mov rax, 0xc000000000000000
	ror rax, 1
	; rax: 0x6000000000000000
	; flags: PF IF OF

	mov rax, 0
	ror rax, 1
	; rax: 0x0000000000000000
	; PF IF

	; ror r/m8, imm
	mov r8, 0x1122334455667788
	ror r8b, 18
	; r8: 0x1122334455667722
	; flags: PF IF

	mov r8, 0x1122334455667788
	push r8
	ror byte[rsp], 18
	; [rsp]: 0x1122334455667722
	; flags: PF IF OF

	; ror r/m8, cl
	mov r8, 0x1122334455667788
	mov cl, 2
	ror r8b, cl
	; r8: 0x1122334455667722
	; flags: PF IF OF

	mov r8, 0x1122334455667788
	push r8
	mov cl, 2 
	ror byte[rsp], cl
	; [rsp]: 0x1122334455667722
	; flags: PF IF OF
	
	; ror r/m16, imm
	mov r8, 0x1122334455667788
	ror r8w, 23
	; r8: 0x11223344556610ef
	; flags: PF IF OF

	mov r8, 0x1122334455667788
	push r8
	ror word [rsp], 23
	; [rsp[: 0x11223344556610ef
	; flags: PF IF

	; ror r/m16, cl
	mov r8, 0x1122334455667788
	mov cl, 8
	ror r8w, cl
	; r8: 0x1122334455668877
	; flags: CF PF IF

	mov r8, 0x1122334455667788
	push r8
	mov cl, 8
	ror word [rsp], cl
	; [rsp]: 0x1122334455668877
	; flags: CF PF IF
	
	; ror r/m32, imm
	mov r8, 0x1122334455667788
	ror r8d, 37
	; r8:  0x42ab33bc
	; flags: PF IF

	mov r8, 0x1122334455667788
	push r8
	ror dword [rsp], 37
	; [rsp]: 0x1122334442ab33bc
	; flags: PF IF

	; ror r/m32, cl
	mov r8, 0x1122334455667788
	mov cl, 15
	ror r8d, cl
	; r8: 0xef10aacc
	; flags: CF PF IF

	mov r8, 0x1122334455667788
	push r8
	mov cl, 15
	ror dword [rsp], cl
	; [rsp]: 0x11223344ef10aacc
	; flags: CF PF IF

	; ror r/m64, imm
	mov r8, 0x1122334455667788
	ror r8, 49
	; r8:  0x19a22ab33bc40891
	; flags: PF IF

	mov r8, 0x1122334455667788
	push r8
	ror qword [rsp], 49
	; [rsp]: 0x19a22ab33bc40891
	; flags: PF IF
	; ror r/m64, cl

	mov r8, 0x1122334455667788
	mov cl, 15
	ror r8, cl
	; r8: 0xef1022446688aacc
	; flags: CF PF IF

	mov r8, 0x1122334455667788
	push r8
	mov cl, 15
	ror qword [rsp], cl
	; [rsp]: 0xef1022446688aacc
	; flags: CF PF IF

	db 0xcc
