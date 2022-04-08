    .file "platform_asm.s"
	.attribute arch, "rv32i2p0_m2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16

    .data
    .section .data.stack_space
    .global stack_space
stack_space:
    .skip 8192

    .text


    .section .text._start,"ax",@progbits
	.globl	_start
	.type	_start, @function
_start:
    lui sp, %hi(stack_space)
    addi sp, sp, %lo(stack_space)
    addi a4, zero, 1024
    slli a4, a4, 2
    addi a4, a4, -4
    add  sp, sp, a4

    jal zero, main


    .section .text.interrupt_handler_asm,"ax",@progbits
	.globl	interrupt_handler_asm
	.type	interrupt_handler_asm, @function

interrupt_handler_asm:
    addi sp, sp, -128
    sw ra, 4(sp)
    sw gp, 8(sp)
    sw tp, 12(sp)
    sw fp, 16(sp)
    sw t0, 20(sp)
    sw t1, 24(sp)
    sw t2, 28(sp)
    sw t3, 32(sp)
    sw t4, 36(sp)
    sw t5, 40(sp)
    sw t6, 44(sp)
    sw a0, 48(sp)
    sw a1, 52(sp)
    sw a2, 56(sp)
    sw a3, 60(sp)
    sw a4, 64(sp)
    sw a5, 68(sp)
    sw a6, 72(sp)
    sw a7, 76(sp)
    sw s0, 80(sp)
    sw s1, 84(sp)
    sw s2, 88(sp)
    sw s3, 92(sp)
    sw s4, 96(sp)
    sw s5, 100(sp)
    sw s6, 104(sp)
    sw s7, 108(sp)
    sw s8, 112(sp)
    sw s9, 116(sp)
    sw s10, 120(sp)
    sw s11, 124(sp)

    lui a0, %hi(interrupt_handler_address)
    addi a0, a0, %lo(interrupt_handler_address)
    lw a0, 0(a0)

    jalr a0

    lw ra, 4(sp)
    lw gp, 8(sp)
    lw tp, 12(sp)
    lw fp, 16(sp)
    lw t0, 20(sp)
    lw t1, 24(sp)
    lw t2, 28(sp)
    lw t3, 32(sp)
    lw t4, 36(sp)
    lw t5, 40(sp)
    lw t6, 44(sp)
    lw a0, 48(sp)
    lw a1, 52(sp)
    lw a2, 56(sp)
    lw a3, 60(sp)
    lw a4, 64(sp)
    lw a5, 68(sp)
    lw a6, 72(sp)
    lw a7, 76(sp)
    lw s0, 80(sp)
    lw s1, 84(sp)
    lw s2, 88(sp)
    lw s3, 92(sp)
    lw s4, 96(sp)
    lw s5, 100(sp)
    lw s6, 104(sp)
    lw s7, 108(sp)
    lw s8, 112(sp)
    lw s9, 116(sp)
    lw s10, 120(sp)
    lw s11, 124(sp)

    addi sp, sp, 128
    mret
