#ifndef instruction_HEADER
#define instruction_HEADER

#include <stdint.h>

enum r5_op {
    R5_ADD,
    R5_ADDI,
    R5_AND,
    R5_ANDI,
    R5_AUIPC,
    R5_BEQ,
    R5_BGE,
    R5_BGEU,
    R5_BLT,
    R5_BLTU,
    R5_BNE,
    R5_CSRRC,
    R5_CSRRCI,
    R5_CSRRS,
    R5_CSRRSI,
    R5_CSRRW,
    R5_CSRRWI,
    R5_EBREAK,
    R5_ECALL,
    R5_FENCE,
    R5_FENCEI,
    R5_JAL,
    R5_JALR,
    R5_LB,
    R5_LBU,
    R5_LH,
    R5_LHU,
    R5_LW,
    R5_LUI,
    R5_MRET,
    R5_MUL,
    R5_MULH,
    R5_MULHSU,
    R5_MULHU,
    R5_DIV,
    R5_DIVU,
    R5_REM,
    R5_REMU,
    R5_OR,
    R5_ORI,
    R5_RDCYCLE,
    R5_RDCYCLEH,
    R5_RDTIME,
    R5_RDTIMEH,
    R5_RDINSTRET,
    R5_RDINSTRETH,
    R5_SB,
    R5_SH,
    R5_SW,
    R5_SLL,
    R5_SLLI,
    R5_SLT,
    R5_SLTI,
    R5_SLTU,
    R5_SLTIU,
    R5_SRA,
    R5_SRAI,
    R5_SRET,
    R5_SRL,
    R5_SRLI,
    R5_SUB,
    R5_URET,
    R5_WFI,
    R5_XOR,
    R5_XORI,
    R5_INVALID_OP
};

#define R5_REGISTER_MAX 31

enum r5_register_x {
    R5_X0,  R5_X1,  R5_X2,  R5_X3,
    R5_X4,  R5_X5,  R5_X6,  R5_X7,
    R5_X8,  R5_X9,  R5_X10, R5_X11,
    R5_X12, R5_X13, R5_X14, R5_X15,
    R5_X16, R5_X17, R5_X18, R5_X19,
    R5_X20, R5_X21, R5_X22, R5_X23,
    R5_X24, R5_X25, R5_X26, R5_X27,
    R5_X28, R5_X29, R5_X30, R5_X31,
    R5_INVALID_REGISTER_X
};

enum r5_register {
    R5_ZERO, R5_RA, R5_SP,  R5_GP,
    R5_TP,   R5_T0, R5_T1,  R5_T2,
    R5_FP,   R5_S1, R5_A0,  R5_A1,
    R5_A2,   R5_A3, R5_A4,  R5_A5,
    R5_A6,   R5_A7, R5_S2,  R5_S3,
    R5_S4,   R5_S5, R5_S6,  R5_S7,
    R5_S8,   R5_S9, R5_S10, R5_S11, 
    R5_T3,   R5_T4, R5_T5,  R5_T6,
    R5_INVALID_REGISTER
};

struct r5_instruction {
    enum r5_register rs2;
    enum r5_register rs1;
    enum r5_register rd;
    uint32_t csr;
    enum r5_op op;
    uint32_t shamt;
    uint32_t immediate;
};


const char * r5_register_string(enum r5_register);
const char * r5_op_string(enum r5_op);


int r5_instruction_snprintf(
    char * buf,
    unsigned int buf_size,
    const struct r5_instruction * ins
);

#endif