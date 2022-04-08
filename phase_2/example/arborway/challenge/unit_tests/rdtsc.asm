BITS 64

start:
	mov rax, 0xffffffffffffffff
	mov rdx, 0xffffffffffffffff
	rdtsc
	; Expected:
	; rax = some value but the upper 32 bits are zeroed
	; rdx = some value but the upper 32 bits are zeroed
	db 0xcc
