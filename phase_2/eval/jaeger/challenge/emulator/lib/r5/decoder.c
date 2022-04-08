#include "result.h"
#include "instruction.h"

#include <stdint.h>
#include <stdio.h>

enum op_type {
    OP_TYPE_B,
    OP_TYPE_R,
    OP_TYPE_I,
    OP_TYPE_S,
    OP_TYPE_U,
    OP_TYPE_UJ,
    OP_TYPE_SPECIAL
};



struct op_code {
    uint32_t funct3;
    uint32_t funct7;
    uint32_t op_code;
    enum r5_op op;
    enum op_type op_type;
};


const struct op_code op_codes[] = {
    {
        .funct7 = 0,
        .funct3 = 0,
        .op_code = 0b0110111,
        .op = R5_LUI,
        .op_type = OP_TYPE_U
    },
    {
        .funct7 = 0,
        .funct3 = 0,
        .op_code = 0b0010111,
        .op = R5_AUIPC,
        .op_type = OP_TYPE_U
    },
    {
        .funct7 = 0,
        .funct3 = 0,
        .op_code = 0b1100111,
        .op = R5_JALR,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0,
        .funct3 = 0,
        .op_code = 0b1100011,
        .op = R5_BEQ,
        .op_type = OP_TYPE_B
    },
    {
        .funct7 = 0,
        .funct3 = 0b001,
        .op_code = 0b1100011,
        .op = R5_BNE,
        .op_type = OP_TYPE_B
    },
    {
        .funct7 = 0,
        .funct3 = 0b100,
        .op_code = 0b1100011,
        .op = R5_BLT,
        .op_type = OP_TYPE_B,
    },
    {
        .funct7 = 0,
        .funct3 = 0b101,
        .op_code = 0b1100011,
        .op = R5_BGE,
        .op_type = OP_TYPE_B,
    },
    {
        .funct7 = 0,
        .funct3 = 0b110,
        .op_code = 0b1100011,
        .op = R5_BLTU,
        .op_type = OP_TYPE_B
    },
    {
        .funct7 = 0,
        .funct3 = 0b111,
        .op_code = 0b1100011,
        .op = R5_BGEU,
        .op_type = OP_TYPE_B,
    },
    {
        .funct7 = 0,
        .funct3 = 0b000,
        .op_code = 0b0000011,
        .op = R5_LB,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0,
        .funct3 = 0b001,
        .op_code = 0b0000011,
        .op = R5_LH,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0,
        .funct3 = 0b010,
        .op_code = 0b0000011,
        .op = R5_LW,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0,
        .funct3 = 0b100,
        .op_code = 0b0000011,
        .op = R5_LBU,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0,
        .funct3 = 0b101,
        .op_code = 0b0000011,
        .op = R5_LHU,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0,
        .funct3 = 0b000,
        .op_code = 0b0100011,
        .op = R5_SB,
        .op_type = OP_TYPE_S
    },
    {
        .funct7 = 0,
        .funct3 = 0b001,
        .op_code = 0b0100011,
        .op = R5_SH,
        .op_type = OP_TYPE_S
    },
    {
        .funct7 = 0,
        .funct3 = 0b010,
        .op_code = 0b0100011,
        .op = R5_SW,
        .op_type = OP_TYPE_S
    },
    {
        .funct7 = 0,
        .funct3 = 0b000,
        .op_code = 0b0010011,
        .op = R5_ADDI,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0,
        .funct3 = 0b010,
        .op_code = 0b0010011,
        .op = R5_SLTI,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0,
        .funct3 = 0b011,
        .op_code = 0b0010011,
        .op = R5_SLTIU,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0,
        .funct3 = 0b100,
        .op_code = 0b0010011,
        .op = R5_XORI,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0,
        .funct3 = 0b110,
        .op_code = 0b0010011,
        .op = R5_ORI,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0,
        .funct3 = 0b111,
        .op_code = 0b0010011,
        .op = R5_ANDI,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0,
        .funct3 = 0b001,
        .op_code = 0b0010011,
        .op = R5_SLLI,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0b0100000,
        .funct3 = 0b101,
        .op_code = 0b0010011,
        .op = R5_SRAI,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0,
        .funct3 = 0b101,
        .op_code = 0b0010011,
        .op = R5_SRLI,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0b0000000,
        .funct3 = 0b000,
        .op_code = 0b0110011,
        .op = R5_ADD,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0100000,
        .funct3 = 0b000,
        .op_code = 0b0110011,
        .op = R5_SUB,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0000000,
        .funct3 = 0b001,
        .op_code = 0b0110011,
        .op = R5_SLL,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0000000,
        .funct3 = 0b010,
        .op_code = 0b0110011,
        .op = R5_SLT,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0000000,
        .funct3 = 0b011,
        .op_code = 0b0110011,
        .op = R5_SLTU,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0000000,
        .funct3 = 0b100,
        .op_code = 0b0110011,
        .op = R5_XOR,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0000000,
        .funct3 = 0b101,
        .op_code = 0b0010011,
        .op = R5_SRL,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0100000,
        .funct3 = 0b101,
        .op_code = 0b0010011,
        .op = R5_SRA,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0000000,
        .funct3 = 0b110,
        .op_code = 0b0110011,
        .op = R5_OR,
        .op_type = OP_TYPE_R,
    },
    {
        .funct7 = 0b0000000,
        .funct3 = 0b111,
        .op_code = 0b0110011,
        .op = R5_AND,
        .op_type = OP_TYPE_R,
    },
    {
        .funct7 = 0,
        .funct3 = 0b000,
        .op_code = 0b0001111,
        .op = R5_FENCE,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0,
        .funct3 = 0b001,
        .op_code = 0b0001111,
        .op = R5_FENCEI,
        .op_type = OP_TYPE_I
    },
    {
        .funct7 = 0,
        .funct3 = 0b000,
        .op_code = 0b1110011,
        .op = R5_ECALL,
        .op_type = OP_TYPE_SPECIAL
    },
    {
        .funct7 = 0b0001000,
        .funct3 = 0b000,
        .op_code = 0b1110011,
        .op = R5_SRET,
        .op_type = OP_TYPE_SPECIAL
    },
    {
        .funct7 = 0b0011000,
        .funct3 = 0b000,
        .op_code = 0b1110011,
        .op = R5_MRET,
        .op_type = OP_TYPE_SPECIAL
    },
    {
        .funct7 = 0,
        .funct3 = 0b001,
        .op_code = 0b1110011,
        .op = R5_CSRRW,
        .op_type = OP_TYPE_SPECIAL
    },
    {
        .funct7 = 0,
        .funct3 = 0b010,
        .op_code = 0b1110011,
        .op = R5_CSRRS,
        .op_type = OP_TYPE_SPECIAL
    },
    {
        .funct7 = 0,
        .funct3 = 0b011,
        .op_code = 0b1110011,
        .op = R5_CSRRC,
        .op_type = OP_TYPE_SPECIAL
    },
    {
        .funct7 = 0,
        .funct3 = 0b101,
        .op_code = 0b1110011,
        .op = R5_CSRRWI,
        .op_type = OP_TYPE_SPECIAL
    },
    {
        .funct7 = 0,
        .funct3 = 0b110,
        .op_code = 0b1110011,
        .op = R5_CSRRSI,
        .op_type = OP_TYPE_SPECIAL
    },
    {
        .funct7 = 0,
        .funct3 = 0b111,
        .op_code = 0b1110011,
        .op = R5_CSRRCI,
        .op_type = OP_TYPE_SPECIAL
    },
    {
        .funct7 = 0b0000001,
        .funct3 = 0b000,
        .op_code = 0b0110011,
        .op = R5_MUL,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0000001,
        .funct3 = 0b001,
        .op_code = 0b0110011,
        .op = R5_MULH,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0000001,
        .funct3 = 0b010,
        .op_code = 0b0110011,
        .op = R5_MULHSU,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0000001,
        .funct3 = 0b011,
        .op_code = 0b0110011,
        .op = R5_MULHU,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0000001,
        .funct3 = 0b100,
        .op_code = 0b0110011,
        .op = R5_DIV,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0000001,
        .funct3 = 0b101,
        .op_code = 0b0110011,
        .op = R5_DIVU,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0000001,
        .funct3 = 0b110,
        .op_code = 0b0110011,
        .op = R5_REM,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0000001,
        .funct3 = 0b111,
        .op_code = 0b0110011,
        .op = R5_REMU,
        .op_type = OP_TYPE_R
    },
    {
        .funct7 = 0b0001000,
        .funct3 = 0b000,
        .op_code = 0b1110011,
        .op = R5_WFI,
        .op_type = OP_TYPE_SPECIAL
    },
    {
        .funct7 = 0,
        .funct3 = 0,
        .op_code = 0b1101111,
        .op = R5_JAL,
        .op_type = OP_TYPE_UJ
    }
};


#define NUM_OPCODE_ENTRIES (sizeof(op_codes) / sizeof(struct op_code))


int decode_r(struct r5_instruction * ins, enum r5_op op, uint32_t word) {
    ins->rs2 = (word >> 20) & 0x1f;
    ins->rs1 = (word >> 15) & 0x1f;
    ins->rd = (word >> 7) & 0x1f;
    ins->op = op;
    ins->csr = 0;
    ins->shamt = 0;
    ins->immediate = 0;

    return 0;
}

int decode_i(struct r5_instruction * ins, enum r5_op op, uint32_t word) {
    ins->rs2 = R5_INVALID_REGISTER;
    ins->rs1 = (word >> 15) & 0x1f;
    ins->rd = (word >> 7) & 0x1f;
    ins->immediate = (word >> 20) & 0xfff;
    if (ins->immediate & 0x800) {
        ins->immediate |= 0xfffff000;
    }
    ins->op = op;
    ins->csr = 0;
    ins->shamt = 0;

    return 0;
}

int decode_b(struct r5_instruction * ins, enum r5_op op, uint32_t word) {
    ins->rs2 = (word >> 20) & 0x1f;
    ins->rs1 = (word >> 15) & 0x1f;

    ins->immediate = ((word >> 7) & 1) << 11;
    ins->immediate |= ((word >> 8) & 0xf) << 1;
    ins->immediate |= ((word >> 25) & 0x3f) << 5;
    ins->immediate |= ((word >> 31) & 1) << 12;

    if (ins->immediate & 0x1000) {
        ins->immediate |= 0xffffe000;
    }

    ins->op = op;

    ins->rd = R5_INVALID_REGISTER;
    ins->csr = 0;
    ins->shamt = 0;

    return 0;
}

int decode_s(struct r5_instruction * ins, enum r5_op op, uint32_t word) {
    ins->rs2 = (word >> 20) & 0x1f;
    ins->rs1 = (word >> 15) & 0x1f;

    ins->immediate = (word >> 7) & 0x1f;
    ins->immediate |= ((word >> 25) & 0x7f) << 5;
    if (ins->immediate & 0x800) {
        ins->immediate |= 0xfffff000;
    }

    ins->op = op;

    ins->rd = R5_INVALID_REGISTER;
    ins->csr = 0;
    ins->shamt = 0;

    return 0;
}

int decode_u(struct r5_instruction * ins, enum r5_op op, uint32_t word) {
    ins->rd = (word >> 7) & 0x1f;
    ins->immediate = word >> 12;

    ins->op = op;

    ins->rs2 = R5_INVALID_REGISTER;
    ins->rs1 = R5_INVALID_REGISTER;
    ins->csr = 0;
    ins->shamt = 0;

    return 0;
}

int decode_uj(struct r5_instruction * ins, enum r5_op op, uint32_t word) {
    ins->rd = (word >> 7) & 0x1f;
    ins->immediate = word & 0x000ff000;
    ins->immediate |= ((word >> 20) & 1) << 11;
    ins->immediate |= ((word >> 21) & 0x3ff) << 1;
    ins->immediate |= ((word >> 31) & 1) << 20;
    if (ins->immediate & 0x00100000) {
        ins->immediate |= 0xffe00000;
    }

    ins->op = op;

    ins->rs1 = R5_INVALID_REGISTER;
    ins->rs2 = R5_INVALID_REGISTER;
    ins->csr = 0;
    ins->shamt = 0;

    return 0;
}

int decode_special(struct r5_instruction * ins, enum r5_op op, uint32_t word) {
    switch (op) {
    case R5_EBREAK:
    case R5_ECALL:
    case R5_URET:
    case R5_SRET:
    case R5_MRET:
    case R5_WFI: {
        uint32_t funct7 = (word >> 25) & 0x7f;
        uint32_t rs2 = (word >> 20) & 0x1f;
        switch (rs2) {
        case 0: ins->op = R5_ECALL; break;
        case 1: ins->op = R5_EBREAK; break;
        case 2:
            switch (funct7) {
                case 0: ins->op = R5_URET; break;
                case 8: ins->op = R5_SRET; break;
                case 24: ins->op = R5_MRET; break;
                default: return ERROR_INVALID_INSTRUCTION;
            }
            break;
        case 5: ins->op = R5_WFI; break;
        default: return ERROR_INVALID_INSTRUCTION;
        }
        ins->rd = R5_INVALID_REGISTER;
        ins->rs2 = R5_INVALID_REGISTER;
        ins->rs1 = R5_INVALID_REGISTER;
        ins->immediate = 0;
        ins->csr = 0;
        ins->shamt = 0;
        break;
    }
    case R5_CSRRC:
    case R5_CSRRS:
    case R5_CSRRW:
        ins->rs1 = (word >> 15) & 0x1f;
        ins->rd = (word >> 7) & 0x1f;
        ins->csr = (word >> 20) & 0xfff;
        ins->rs2 = R5_INVALID_REGISTER;
        ins->op = op;
        ins->shamt = 0;
        ins->immediate = 0;
        break;
    case R5_CSRRCI:
    case R5_CSRRSI:
    case R5_CSRRWI:
        ins->immediate = (word >> 15) & 0x1f;
        ins->rd = (word >> 7) & 0x1f;
        ins->csr = (word >> 20) & 0xfff;
        ins->rs2 = R5_INVALID_REGISTER;
        ins->rs1 = R5_INVALID_REGISTER;
        ins->shamt = 0;
        ins-> op = op;
        break;
    default:
        return ERROR_INVALID_INSTRUCTION;
    }

    return OK;
}

int r5_decode(struct r5_instruction * ins, uint32_t word) {
    uint32_t opcode = word & 0x7f;
    uint32_t funct3 = (word >> 12) & 0x7;
    uint32_t funct7 = (word >> 25) & 0x7f;

    uint32_t i;

    for (i = 0; i < NUM_OPCODE_ENTRIES; i++) {
        const struct op_code * op_code = &op_codes[i];

        /*
        printf(
            "%s: funct7 0x%x-0x%x, funct3 0x%x-0x%x, opcode 0x%x-0x%x\n",
            r5_op_string(op_code->op),
            funct7,
            op_code->funct7,
            funct3,
            op_code->funct3,
            opcode,
            op_code->op_code
        );
        */

        switch(op_code->op_type) {
        case OP_TYPE_R:
            if (    (op_code->op_code == opcode)
                 && (op_code->funct3 == funct3)
                 && (op_code->funct7 == funct7)) {
                return decode_r(ins, op_code->op, word);
            }
            break;
        case OP_TYPE_I:
            if (    (op_code->op_code == opcode)
                 && (op_code->funct3 == funct3)) {
                return decode_i(ins, op_code->op, word);
            }
            break;
        case OP_TYPE_B:
            if (    (op_code->op_code == opcode)
                 && (op_code->funct3 == funct3)) {
                return decode_b(ins, op_code->op, word);
            }
            break;
        case OP_TYPE_S:
            if (    (op_code->op_code == opcode)
                 && (op_code->funct3 == funct3)) {
                return decode_s(ins, op_code->op, word);
            }
            break;
        case OP_TYPE_U:
            if (op_code->op_code == opcode) {
                return decode_u(ins, op_code->op, word);
            }
            break;
        case OP_TYPE_UJ:
            if (op_code->op_code == opcode) {
                return decode_uj(ins, op_code->op, word);
            }
            break;
        case OP_TYPE_SPECIAL:
            if ((op_code->op_code == opcode) && (op_code->funct3 == funct3)) {
                return decode_special(ins, op_code->op, word);
            }
            break;
        }
    }

    return ERROR_INVALID_INSTRUCTION;
}