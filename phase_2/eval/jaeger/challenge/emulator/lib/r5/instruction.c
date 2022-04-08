#include "instruction.h"

#include <stdio.h>
#include <stdlib.h>

struct identifier_string {
    unsigned int identifier;
    const char * string;
};

struct identifier_string register_strings[] = {
    {R5_ZERO, "zero"}, {R5_RA, "ra"}, {R5_SP, "sp"}, {R5_GP, "gp"},
    {R5_TP, "tp"}, {R5_T0, "t0"}, {R5_T1, "t1"}, {R5_T2, "t2"},
    {R5_FP, "fp"}, {R5_S1, "s1"}, {R5_A0, "a0"}, {R5_A1, "a1"},
    {R5_A2, "a2"}, {R5_A3, "a3"}, {R5_A4, "a4"}, {R5_A5, "a5"},
    {R5_A6, "a6"}, {R5_A7, "a7"}, {R5_S2, "s2"}, {R5_S3, "s3"},
    {R5_S4, "s4"}, {R5_S5, "s5"}, {R5_S6, "s6"}, {R5_S7, "s7"},
    {R5_S8, "s8"}, {R5_S9, "s9"}, {R5_S10, "s10"}, {R5_S11, "s11"},
    {R5_T3, "t3"}, {R5_T4, "t4"}, {R5_T5, "t5"}, {R5_T6, "t6"}
};

struct identifier_string op_strings[] = {
    {R5_ADD, "add"},
    {R5_ADDI, "addi"},
    {R5_AND, "and"},
    {R5_ANDI, "andi"},
    {R5_AUIPC, "auipc"},
    {R5_BEQ, "beq"},
    {R5_BGE, "bge"},
    {R5_BGEU, "bgeu"},
    {R5_BLT, "blt"},
    {R5_BLTU, "bltu"},
    {R5_BNE, "bne"},
    {R5_CSRRC, "csrrc"},
    {R5_CSRRCI, "csrrci"},
    {R5_CSRRS, "csrrs"},
    {R5_CSRRSI, "csrrsi"},
    {R5_CSRRW, "csrrw"},
    {R5_CSRRWI, "csrrwi"},
    {R5_EBREAK, "ebreak"},
    {R5_ECALL, "ecall"},
    {R5_FENCE, "fence"},
    {R5_FENCEI, "fencei"},
    {R5_JAL, "jal"},
    {R5_JALR, "jalr"},
    {R5_LB, "lb"},
    {R5_LBU, "lbu"},
    {R5_LH, "lh"},
    {R5_LHU, "lhu"},
    {R5_LW, "lw"},
    {R5_LUI, "lui"},
    {R5_MRET, "mret"},
    {R5_MUL, "mul"},
    {R5_MULH, "mulh"},
    {R5_MULHSU, "mulhsu"},
    {R5_MULHU, "mulhu"},
    {R5_DIV, "div"},
    {R5_DIVU, "divu"},
    {R5_REM, "rem"},
    {R5_REMU, "remu"},
    {R5_OR, "or"},
    {R5_ORI, "ori"},
    {R5_RDCYCLE, "rdcycle"},
    {R5_RDCYCLEH, "rdcycleh"},
    {R5_RDTIME, "rdtime"},
    {R5_RDTIMEH, "rdtimeh"},
    {R5_RDINSTRET, "rdinstret"},
    {R5_RDINSTRETH, "rdinstreth"},
    {R5_SB, "sb"},
    {R5_SH, "sh"},
    {R5_SW, "sw"},
    {R5_SLL, "sll"},
    {R5_SLLI, "slli"},
    {R5_SLT, "slt"},
    {R5_SLTI, "slti"},
    {R5_SLTU, "sltu"},
    {R5_SLTIU, "sltiu"},
    {R5_SRA, "sra"},
    {R5_SRAI, "srai"},
    {R5_SRET, "sret"},
    {R5_SRL, "srl"},
    {R5_SRLI, "srli"},
    {R5_SUB, "sub"},
    {R5_URET, "uret"},
    {R5_WFI, "wfi"},
    {R5_XOR, "xor"},
    {R5_XORI, "xori"},
};

const char * r5_register_string(enum r5_register reg) {
    unsigned int i;
    for (
        i = 0;
        i < sizeof(register_strings) / sizeof(struct identifier_string);
        i++
    ) {
        if (register_strings[i].identifier == reg) {
            return register_strings[i].string;
        }
    }
    return NULL;
}

const char * r5_op_string(enum r5_op op) {
    unsigned int i;
    for (
        i = 0;
        i < sizeof(op_strings) / sizeof(struct identifier_string);
        i++
    ) {
        if (op_strings[i].identifier == op) {
            return op_strings[i].string;
        }
    }
    return NULL;
}

int r5_instruction_snprintf(
    char * buf,
    unsigned int buf_size,
    const struct r5_instruction * ins
) {
    switch (ins->op) {
    /* Type I */
    case R5_JALR:
    case R5_LB:
    case R5_LBU:
    case R5_LH:
    case R5_LHU:
    case R5_LW:
    case R5_ADDI:
    case R5_SLTI:
    case R5_SLTIU:
    case R5_XORI:
    case R5_ORI:
    case R5_ANDI:
    case R5_SLLI:
    case R5_SRLI:
    case R5_SRAI:
    case R5_FENCE:
    case R5_FENCEI:
        return snprintf(
            buf,
            buf_size,
            "%s %s, %s, 0x%x",
            r5_op_string(ins->op),
            r5_register_string(ins->rd),
            r5_register_string(ins->rs1),
            ins->immediate
        );
    /* Op Type B */
    case R5_BEQ:
    case R5_BNE:
    case R5_BLT:
    case R5_BGE:
    case R5_BLTU:
    case R5_BGEU:
        return snprintf(
            buf,
            buf_size,
            "%s %s, %s, 0x%x",
            r5_op_string(ins->op),
            r5_register_string(ins->rs1),
            r5_register_string(ins->rs2),
            ins->immediate
        );
    /* Op Type S */
    case R5_SB:
    case R5_SH:
    case R5_SW:
        return snprintf(
            buf,
            buf_size,
            "%s %s, 0x%x(%s)",
            r5_op_string(ins->op),
            r5_register_string(ins->rs2),
            ins->immediate,
            r5_register_string(ins->rs1)
        );
    /* Type R */
    case R5_ADD:
    case R5_SUB:
    case R5_SLL:
    case R5_SLT:
    case R5_SLTU:
    case R5_XOR:
    case R5_SRL:
    case R5_SRA:
    case R5_OR:
    case R5_AND:
    case R5_MUL:
    case R5_MULH:
    case R5_MULHSU:
    case R5_MULHU:
    case R5_DIV:
    case R5_DIVU:
    case R5_REM:
    case R5_REMU:
        return snprintf(
            buf,
            buf_size,
            "%s %s, %s, %s",
            r5_op_string(ins->op),
            r5_register_string(ins->rd),
            r5_register_string(ins->rs1),
            r5_register_string(ins->rs2)
        );
    case R5_LUI:
    case R5_AUIPC:
        return snprintf(
            buf,
            buf_size,
            "%s %s, 0x%x",
            r5_op_string(ins->op),
            r5_register_string(ins->rd),
            ins->immediate
        );
    case R5_JAL:
        return snprintf(
            buf,
            buf_size,
            "%s %s, 0x%x",
            r5_op_string(ins->op),
            r5_register_string(ins->rd),
            ins->immediate
        );
    /* Type Special */
    case R5_EBREAK:
    case R5_ECALL:
    case R5_MRET:
    case R5_SRET:
    case R5_URET:
    case R5_WFI:
        return snprintf(buf, buf_size, "%s", r5_op_string(ins->op));
    case R5_CSRRW:
    case R5_CSRRS:
    case R5_CSRRC:
        return snprintf(
            buf,
            buf_size,
            "%s %s, 0x%x, %s",
            r5_op_string(ins->op),
            r5_register_string(ins->rd),
            ins->csr,
            r5_register_string(ins->rs1)
        );
    case R5_CSRRWI:
    case R5_CSRRSI:
    case R5_CSRRCI:
        return snprintf(
            buf,
            buf_size,
            "%s %s, 0x%x, 0x%x",
            r5_op_string(ins->op),
            r5_register_string(ins->rd),
            ins->csr,
            ins->immediate
        );
    case R5_RDCYCLE:
    case R5_RDCYCLEH:
    case R5_RDTIME:
    case R5_RDTIMEH:
    case R5_RDINSTRET:
    case R5_RDINSTRETH:
    case R5_INVALID_OP:
        break;
    }

    return -1;
}