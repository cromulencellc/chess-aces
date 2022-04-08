BITS 64

start:
	mov rax, 63
	sub rsp, 0x200
	mov rdi, rsp
	syscall
	; rax: 0
	; rdi -> "Linux"
	; rdi+0x41 -> "ubuntu-bionic"
	; rdi+0x82 -> "4.15.0-99-generic"
	; "#100-Ubuntu SMP Wed Apr 22 20:32:56 UTC 2020"
	; "x86_64"
	
	mov rbx, rax
	db 0xcc
