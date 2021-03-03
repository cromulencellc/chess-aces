
#ifndef instructions_HEADER
#define instructions_HEADER
#include <stdint.h>
#include "container/list.h"
#define OP_ADD 0x00
#define OP_SUB 0x01
#define OP_MUL 0x02
#define OP_UDIV 0x03
#define OP_SDIV 0x04
#define OP_UMOD 0x05
#define OP_SMOD 0x06
#define OP_AND 0x07
#define OP_OR 0x08
#define OP_XOR 0x09
#define OP_SHL 0x0A
#define OP_SHR 0x0B
#define OP_ASR 0x0C
#define OP_CMP 0x0D
#define OP_MOV 0x0E
#define OP_CALL 0x10
#define OP_RET 0x11
#define OP_JMP 0x12
#define OP_JE 0x13
#define OP_JL 0x14
#define OP_JLE 0x15
#define OP_JB 0x16
#define OP_JBE 0x17
#define OP_PUSH 0x18
#define OP_POP 0x19
#define OP_STORE 0x1A
#define OP_STOREB 0x1B
#define OP_LOAD 0x1C
#define OP_LOADB 0x1D
#define OP_SYSCALL 0x1E
#define OPERAND_NONE 0x0
#define OPERAND_R 0x1
#define OPERAND_IMM8 0x2
#define OPERAND_IMM16 0x3
#define OPERAND_R_R 0x4
#define OPERAND_R_IMM8 0x5
#define OPERAND_R_IMM16 0x6
#define OPERAND_R_MEM 0x7
#define R0 0
#define R1 1
#define R2 2
#define R3 3
#define R4 4
#define R5 5
#define R6 6
#define R7 7
#define SP R7
#define PC 16
enum operand_type { OPERAND_REGISTER, OPERAND_IMMEDIATE, OPERAND_MEMORY };
struct memop {
  uint8_t base_reg;
  uint8_t index_reg;
  int16_t offset;
};
struct operand {
  enum operand_type type;
  union {
    uint8_t reg;
    uint16_t imm;
    struct memop mem;
  };
};
struct instruction {
  uint16_t address;
  uint32_t length;
  uint8_t opcode;
  uint8_t operand_type;
  uint8_t num_operands;
  struct operand operands[2];
};
int instruction_sprintf(char *buf, uint32_t buf_size,
                        const struct instruction *instruction);
int instruction_decode(struct instruction *instruction, uint16_t address,
                       const uint8_t *bytes, uint32_t bytes_length);
int block_decode(struct list **list, uint16_t address, const uint8_t *bytes,
                 uint32_t bytes_length);
#endif