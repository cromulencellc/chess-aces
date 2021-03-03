#include "instructions.h"
#include <stdio.h>
#include <stdlib.h>
#include "error.h"
const char *register_string(uint8_t reg) {
  switch (reg) {
  case R0:
    return "r0";
  case R1:
    return "r1";
  case R2:
    return "r2";
  case R3:
    return "r3";
  case R4:
    return "r4";
  case R5:
    return "r5";
  case R6:
    return "r6";
  case R7:
    return "r7";
  default:
    return "invalid";
  }
}
const char *mnemonic_string(uint8_t opcode) {
  switch (opcode) {
  case OP_ADD:
    return "add";
  case OP_SUB:
    return "sub";
  case OP_MUL:
    return "mul";
  case OP_UDIV:
    return "udiv";
  case OP_UMOD:
    return "umod";
  case OP_SDIV:
    return "sdiv";
  case OP_SMOD:
    return "smod";
  case OP_AND:
    return "and";
  case OP_OR:
    return "or";
  case OP_XOR:
    return "xor";
  case OP_SHL:
    return "shl";
  case OP_SHR:
    return "shr";
  case OP_ASR:
    return "asr";
  case OP_CMP:
    return "cmp";
  case OP_MOV:
    return "mov";
  case OP_CALL:
    return "call";
  case OP_RET:
    return "ret";
  case OP_JMP:
    return "jmp";
  case OP_JE:
    return "je";
  case OP_JL:
    return "jl";
  case OP_JLE:
    return "jle";
  case OP_JB:
    return "jb";
  case OP_JBE:
    return "jbe";
  case OP_PUSH:
    return "push";
  case OP_POP:
    return "pop";
  case OP_STORE:
    return "store";
  case OP_STOREB:
    return "storeb";
  case OP_LOAD:
    return "load";
  case OP_LOADB:
    return "loadb";
  case OP_SYSCALL:
    return "syscall";
  default:
    return "invalid";
  }
}
int instruction_sprintf(char *buf, uint32_t buf_size,
                        const struct instruction *instruction) {
  switch (instruction->operand_type) {
  case OPERAND_NONE:
    return snprintf(buf, buf_size, "%s", mnemonic_string(instruction->opcode));
  case OPERAND_R:
    return snprintf(buf, buf_size, "%s %s",
                    mnemonic_string(instruction->opcode),
                    register_string(instruction->operands[0].reg));
  case OPERAND_IMM8:
  case OPERAND_IMM16:
    return snprintf(buf, buf_size, "%s 0x%04x",
                    mnemonic_string(instruction->opcode),
                    instruction->operands[0].imm);
  case OPERAND_R_R:
    return snprintf(buf, buf_size, "%s %s,%s",
                    mnemonic_string(instruction->opcode),
                    register_string(instruction->operands[0].reg),
                    register_string(instruction->operands[1].reg));
  case OPERAND_R_IMM8:
  case OPERAND_R_IMM16:
    return snprintf(buf, buf_size, "%s %s, 0x%04x",
                    mnemonic_string(instruction->opcode),
                    register_string(instruction->operands[0].reg),
                    instruction->operands[1].imm);
  case OPERAND_R_MEM:
    return snprintf(buf, buf_size, "%s %s,[%s+%s+0x%x]",
                    mnemonic_string(instruction->opcode),
                    register_string(instruction->operands[0].reg),
                    register_string(instruction->operands[1].mem.base_reg),
                    register_string(instruction->operands[1].mem.index_reg),
                    instruction->operands[1].mem.offset);
  }
  return -1;
}
uint8_t OPERAND_INSTRUCTION_SIZE[8] = {
    1, 2, 2, 3, 2, 3, 4, 4,
};
int instruction_decode_operands(struct instruction *instruction,
                                const uint8_t *bytes) {
  instruction->operand_type = bytes[0] & 7;
  switch (bytes[0] & 7) {
  case OPERAND_NONE:
    instruction->num_operands = 0;
    break;
  case OPERAND_R:
    instruction->operands[0].reg = bytes[1] & 0xf;
    instruction->operands[0].type = OPERAND_REGISTER;
    instruction->num_operands = 1;
    break;
  case OPERAND_IMM8:
    instruction->operands[0].imm = bytes[1];
    if (instruction->operands[0].imm & 0x80) {
      instruction->operands[0].imm |= 0xff00;
    }
    instruction->operands[0].type = OPERAND_IMMEDIATE;
    instruction->num_operands = 1;
    break;
  case OPERAND_IMM16:
    instruction->operands[0].imm = bytes[1];
    instruction->operands[0].imm <<= 8;
    instruction->operands[0].imm |= bytes[2];
    instruction->operands[0].type = OPERAND_IMMEDIATE;
    instruction->num_operands = 1;
    break;
  case OPERAND_R_R:
    instruction->operands[0].reg = (bytes[1] >> 4) & 0xf;
    instruction->operands[0].type = OPERAND_REGISTER;
    instruction->operands[1].reg = bytes[1] & 0xf;
    instruction->operands[1].type = OPERAND_REGISTER;
    instruction->num_operands = 2;
    break;
  case OPERAND_R_IMM8:
    instruction->operands[0].reg = bytes[1] & 0xf;
    instruction->operands[0].type = OPERAND_REGISTER;
    instruction->operands[1].imm = bytes[2];
    if (instruction->operands[1].imm & 0x80) {
      instruction->operands[1].imm |= 0xff00;
    }
    instruction->operands[1].type = OPERAND_IMMEDIATE;
    instruction->num_operands = 2;
    break;
  case OPERAND_R_IMM16:
    instruction->operands[0].reg = bytes[1] & 0xf;
    instruction->operands[0].type = OPERAND_REGISTER;
    instruction->operands[1].imm = bytes[2];
    instruction->operands[1].imm <<= 8;
    instruction->operands[1].imm |= bytes[3];
    instruction->operands[1].type = OPERAND_IMMEDIATE;
    instruction->num_operands = 2;
    break;
  case OPERAND_R_MEM:
    instruction->operands[0].reg = bytes[1] & 0xf;
    instruction->operands[0].type = OPERAND_REGISTER;
    instruction->operands[1].mem.base_reg = (bytes[2] >> 4) & 0xf;
    instruction->operands[1].mem.index_reg = bytes[2] & 0xf;
    instruction->operands[1].mem.offset = bytes[3];
    if (instruction->operands[1].mem.offset & 0x80) {
      instruction->operands[1].mem.offset |= 0xff00;
    }
    instruction->operands[1].type = OPERAND_MEMORY;
    instruction->num_operands = 0;
    break;
  }
  return 0;
}
int instruction_decode(struct instruction *instruction, uint16_t address,
                       const uint8_t *bytes, uint32_t bytes_length) {
  if ((bytes_length < 1) ||
      (bytes_length < OPERAND_INSTRUCTION_SIZE[bytes[0] & 7])) {
    return ERROR_INSTRUCTION_DECODE;
  }
  instruction->address = address;
  instruction->length = OPERAND_INSTRUCTION_SIZE[bytes[0] & 7];
  instruction->opcode = (bytes[0] >> 3) & 0x1f;
  uint8_t operand_bits = bytes[0] & 7;
  switch ((bytes[0] >> 3) & 0x1f) {
  case OP_ADD:
  case OP_SUB:
  case OP_MUL:
  case OP_UDIV:
  case OP_SDIV:
  case OP_UMOD:
  case OP_SMOD:
  case OP_AND:
  case OP_OR:
  case OP_XOR:
  case OP_SHL:
  case OP_SHR:
  case OP_ASR:
  case OP_CMP:
  case OP_MOV: {
    switch (operand_bits) {
    case OPERAND_R_R:
    case OPERAND_R_IMM8:
    case OPERAND_R_IMM16:
      return instruction_decode_operands(instruction, bytes);
    default:
      return ERROR_INSTRUCTION_DECODE;
    }
  }
  case OP_CALL:
  case OP_JMP:
  case OP_JE:
  case OP_JL:
  case OP_JLE:
  case OP_JB:
  case OP_JBE:
  case OP_PUSH: {
    switch (operand_bits) {
    case OPERAND_R:
    case OPERAND_IMM8:
    case OPERAND_IMM16:
      return instruction_decode_operands(instruction, bytes);
    default:
      return ERROR_INSTRUCTION_DECODE;
    }
  }
  case OP_POP: {
    switch (operand_bits) {
    case OPERAND_R:
      return instruction_decode_operands(instruction, bytes);
    default:
      return ERROR_INSTRUCTION_DECODE;
    }
  }
  case OP_STORE:
  case OP_STOREB:
  case OP_LOAD:
  case OP_LOADB: {
    switch (operand_bits) {
    case OPERAND_R_R:
    case OPERAND_R_IMM8:
    case OPERAND_R_IMM16:
    case OPERAND_R_MEM:
      return instruction_decode_operands(instruction, bytes);
    default:
      return ERROR_INSTRUCTION_DECODE;
    }
  }
  case OP_RET:
  case OP_SYSCALL: {
    switch (operand_bits) {
    case OPERAND_NONE:
      return instruction_decode_operands(instruction, bytes);
    default:
      return ERROR_INSTRUCTION_DECODE;
    }
  }
  default:
    return ERROR_INSTRUCTION_DECODE;
  }
}
int block_decode(struct list **list, uint16_t address, const uint8_t *bytes,
                 uint32_t bytes_length) {
  *list = list_create();
  while (1) {
    struct instruction *instruction =
        (struct instruction *)malloc(sizeof(struct instruction));
    int err = instruction_decode(instruction, address, bytes, bytes_length);
    if (err != SUCCESS) {
      list_delete(*list, free);
      free(instruction);
      return err;
    }
    list_append(*list, instruction);
    switch (instruction->opcode) {
    case OP_CALL:
    case OP_RET:
    case OP_JMP:
    case OP_JE:
    case OP_JL:
    case OP_JLE:
    case OP_JB:
    case OP_JBE:
    case OP_SYSCALL:
      break;
    }
    address += instruction->length;
    bytes = &(bytes[instruction->length]);
    bytes_length -= instruction->length;
  }
  return 0;
}