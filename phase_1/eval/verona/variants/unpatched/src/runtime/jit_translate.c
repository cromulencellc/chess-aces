#include "jit.h"
#include <stddef.h>
#include <string.h>
#include "container/bb.h"
#include "error.h"
#include "instructions.h"
#include "rust.h"
#include "target_x86.h"
int get_pointer_to_data_region(struct bb *bb, uint8_t x86_reg) {
  try
    (get_memory_regions(bb, x86_reg)) try
      (add_r64_imm32(bb, x86_reg,
                     offsetof(struct memory_region, data))) return 0;
}
int get_memory_operand_address(struct bb *bb, uint8_t x86_reg,
                               const struct memop *mem) {
  try
    (get_pointer_to_data_region(bb, x86_reg)) try
      (get_register(bb, RAX, mem->base_reg)) try
        (add_r64_r64(bb, x86_reg, RAX)) try
          (get_register(bb, RAX, mem->index_reg)) try
            (add_r64_r64(bb, x86_reg, RAX)) try
              (mov_r64_imm64(bb, RAX, mem->offset)) try
                (add_r64_r64(bb, x86_reg, RAX)) return SUCCESS;
}
int jit_load_operand(struct bb *bb, uint8_t x86_reg,
                     const struct operand *operand) {
  switch (operand->type) {
  case OPERAND_REGISTER:
    try
      (get_register(bb, x86_reg, operand->reg)) break;
  case OPERAND_IMMEDIATE: {
    try
      (mov_r64_imm64(bb, x86_reg, operand->imm)) break;
  }
  case OPERAND_MEMORY: {
    try
      (get_memory_operand_address(bb, x86_reg, &operand->mem)) try
        (bb_push(bb, PREFIX_16)) try
          (bb_push(bb, MOV_R32_RM32)) try
            (modrm_reg_indirect(bb, x86_reg, x86_reg)) break;
  }
  }
  return SUCCESS;
}
int set_branch_flag(struct bb *bb) {
  try
    (get_flags(bb, RAX)) try
      (mov_r32_imm32(bb, RBX, FLAG_BRANCH_SET)) try
        (or_r64_r64(bb, RAX, RBX)) try
          (set_flags(bb, RAX)) return 0;
}
int set_branch_address_operand(struct bb *bb, struct instruction *instruction,
                               struct operand *operand) {
  if (operand->type == OPERAND_IMMEDIATE) {
    uint16_t target = instruction->address + operand->imm;
    try
      (mov_r32_imm32(bb, RAX, target)) try
        (set_branch_address(bb, RAX)) return 0;
  } else {
    try
      (get_register(bb, RAX, operand->reg)) try
        (set_branch_address(bb, RAX))
  }
  return 0;
}
int jit_push_reg(struct bb *bb, uint8_t reg) {
  try
    (get_sp(bb, RAX)) try
      (add_r64_sext_imm8(bb, RAX, -2)) try
        (set_sp(bb, RAX)) try
          (get_pointer_to_data_region(bb, RBX)) try
            (add_r64_r64(bb, RAX, RBX)) try
              (bb_push(bb, PREFIX_16)) try
                (bb_push(bb, MOV_RM32_R32)) try
                  (modrm_reg_indirect(bb, RAX, reg)) return 0;
}
int jit_push_imm(struct bb *bb, uint16_t value) {
  try
    (get_sp(bb, RAX)) try
      (add_r64_sext_imm8(bb, RAX, -2)) try
        (set_sp(bb, RAX)) try
          (get_pointer_to_data_region(bb, RBX)) try
            (add_r64_r64(bb, RAX, RBX)) try
              (mov_r32_imm32(bb, RBX, value)) try
                (bb_push(bb, PREFIX_16)) try
                  (bb_push(bb, MOV_RM32_R32)) try
                    (modrm_reg_indirect(bb, RAX, RBX)) return 0;
}
int jit_pop_reg(struct bb *bb, uint8_t reg) {
  try
    (get_sp(bb, RAX)) try
      (get_pointer_to_data_region(bb, RBX)) try
        (add_r64_r64(bb, RBX, RAX)) try
          (bb_push(bb, PREFIX_16)) try
            (bb_push(bb, MOV_R32_RM32)) try
              (modrm_reg_indirect(bb, RBX, reg)) try
                (add_r64_sext_imm8(bb, RAX, 2)) try
                  (set_sp(bb, RAX)) return 0;
}
int inc_pc(struct bb *bb, const struct instruction *instruction) {
  try
    (get_pc(bb, RAX)) try
      (add_r64_sext_imm8(bb, RAX, instruction->length)) try
        (set_pc(bb, RAX)) return 0;
}
int jit_translate(struct bb *bb, struct instruction *instruction) {
  bb_append(bb,
            "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90",
            16);
  try
    (block_preamble(bb)) try
      (inc_pc(bb, instruction)) switch (instruction->opcode) {
      case OP_ADD:
      case OP_SUB:
      case OP_AND:
      case OP_OR:
      case OP_XOR:
      case OP_SHL:
      case OP_SHR:
      case OP_ASR: {
        if (instruction->opcode == OP_ASR) {
          try
            (get_register_signed(bb, RDX, instruction->operands[0].reg))
        } else {
          try
            (jit_load_operand(bb, RDX, &instruction->operands[0]))
        }
        try
          (jit_load_operand(bb, RCX, &instruction->operands[1])) switch (
              instruction->opcode) {
          case OP_ADD:
            try
              (add_r64_r64(bb, RDX, RCX)) break;
          case OP_SUB:
            try
              (sub_r64_r64(bb, RDX, RCX)) break;
          case OP_AND:
            try
              (and_r64_r64(bb, RDX, RCX)) break;
          case OP_OR:
            try
              (or_r64_r64(bb, RDX, RCX)) break;
          case OP_XOR:
            try
              (xor_r64_r64(bb, RDX, RCX)) break;
          case OP_SHL:
            try
              (shl_r64_cl(bb, RDX)) break;
          case OP_SHR:
            try
              (shr_r64_cl(bb, RDX)) break;
          case OP_ASR:
            try
              (asr_r64_cl(bb, RDX)) break;
          }
        try
          (set_register(bb, RDX, instruction->operands[0].reg)) break;
      }
      case OP_MOV: {
        try
          (jit_load_operand(bb, RCX, &instruction->operands[1])) try
            (set_register(bb, RCX, instruction->operands[0].reg)) break;
      }
      case OP_MUL: {
        try
          (jit_load_operand(bb, RDX, &instruction->operands[0])) try
            (jit_load_operand(bb, RCX, &instruction->operands[1])) try
              (mov_r64_r64(bb, RAX, RDX)) try
                (mul_r64(bb, RCX)) try
                  (set_register(bb, RAX, instruction->operands[0].reg)) break;
      }
      case OP_UDIV:
      case OP_UMOD:
        try
          (jit_load_operand(bb, RBX, &instruction->operands[0])) try
            (jit_load_operand(bb, RCX, &instruction->operands[1])) try
              (mov_r64_r64(bb, RAX, RBX)) try
                (mov_r32_imm32(bb, RDX, 0)) try
                  (div_r64(bb, RCX)) switch (instruction->opcode) {
                  case OP_UDIV:
                    try
                      (set_register(bb, RAX,
                                    instruction->operands[0].reg)) break;
                  case OP_UMOD:
                    try
                      (set_register(bb, RDX,
                                    instruction->operands[0].reg)) break;
                  }
        break;
      case OP_SDIV:
      case OP_SMOD:
        try
          (jit_load_operand(bb, RBX, &instruction->operands[0])) try
            (jit_load_operand(bb, RCX, &instruction->operands[1])) try
              (mov_r64_r64(bb, RAX, RBX)) try
                (mov_r64_imm64(bb, RDX, 0)) try
                  (idiv_r64(bb, RCX)) switch (instruction->opcode) {
                  case OP_SDIV:
                    try
                      (set_register(bb, RAX,
                                    instruction->operands[0].reg)) break;
                  case OP_SMOD:
                    try
                      (set_register(bb, RDX,
                                    instruction->operands[0].reg)) break;
                  }
        break;
      case OP_CMP: {
        try
          (jit_load_operand(bb, RBX, &instruction->operands[0])) try
            (jit_load_operand(bb, RCX, &instruction->operands[1])) try
              (cmp_flags_sequence(bb, RBX, RCX, RDX)) try
                (get_flags(bb, RAX)) try
                  (and_r64_sext_imm32(bb, RAX,
                                      ~(FLAG_EQUAL | FLAG_LESS_THAN_UNSIGNED |
                                        FLAG_LESS_THAN_SIGNED))) try
                    (or_r64_r64(bb, RAX, RDX)) try
                      (set_flags(bb, RAX)) break;
      }
      case OP_CALL: {
        uint16_t ret = instruction->address + instruction->length;
        try
          (jit_push_imm(bb, ret)) try
            (set_branch_address_operand(bb, instruction,
                                        &instruction->operands[0])) try
              (set_branch_flag(bb)) break;
      }
      case OP_RET: {
        try
          (jit_pop_reg(bb, RCX)) try
            (set_branch_address(bb, RCX)) try
              (set_branch_flag(bb)) break;
      }
      case OP_JMP: {
        try
          (set_branch_address_operand(bb, instruction,
                                      &instruction->operands[0])) try
            (set_branch_flag(bb)) break;
      }
      case OP_JE:
      case OP_JL:
      case OP_JLE:
      case OP_JB:
      case OP_JBE: {
        try
          (mov_r32_imm32(bb, RDX, 0)) if ((instruction->opcode == OP_JE) ||
                                          (instruction->opcode == OP_JLE) ||
                                          (instruction->opcode == OP_JBE)) {
            try
              (get_flags(bb, RAX)) try
                (mov_r32_imm32(bb, RCX, FLAG_EQUAL)) try
                  (and_r64_r64(bb, RAX, RCX)) try
                    (cmp_r64_r64(bb, RAX, RCX)) try
                      (bb_append(bb, "\x75\x02\xb2\x01", 4))
          }
        if ((instruction->opcode == OP_JL) || (instruction->opcode == OP_JLE)) {
          try
            (get_flags(bb, RAX)) try
              (mov_r32_imm32(bb, RCX, FLAG_LESS_THAN_SIGNED)) try
                (and_r64_r64(bb, RAX, RCX)) try
                  (cmp_r64_r64(bb, RAX, RCX)) try
                    (bb_append(bb, "\x75\x02\xb2\x01", 4))
        }
        if ((instruction->opcode == OP_JB) || (instruction->opcode == OP_JBE)) {
          try
            (get_flags(bb, RAX)) try
              (mov_r32_imm32(bb, RCX, FLAG_LESS_THAN_UNSIGNED)) try
                (and_r64_r64(bb, RAX, RCX)) try
                  (cmp_r64_r64(bb, RAX, RCX)) try
                    (bb_append(bb, "\x75\x02\xb2\x01", 4))
        }
        try
          (mov_r32_imm32(bb, RCX, 1)) try
            (cmp_r64_r64(bb, RCX, RDX)) try
              (bb_push(bb, 0x75)) uint32_t branch_offset = bb_length(bb);
        try
          (bb_push(bb, 0xff)) try
            (set_branch_address_operand(bb, instruction,
                                        &instruction->operands[0])) try
              (set_branch_flag(bb)) uint32_t post_set_branch_address_offset =
                  bb_length(bb);
        uint8_t branch_distance =
            post_set_branch_address_offset - branch_offset;
        ((uint8_t *)bb_data(bb))[branch_offset] = branch_distance;
        break;
      }
      case OP_PUSH: {
        try
          (jit_load_operand(bb, RCX, &instruction->operands[0])) try
            (jit_push_reg(bb, RCX)) break;
      }
      case OP_POP: {
        try
          (jit_pop_reg(bb, RCX)) try
            (set_register(bb, RCX, instruction->operands[0].reg)) break;
      }
      case OP_STORE:
        try
          (jit_load_operand(
              bb, RCX,
              &instruction->operands[0])) if ((instruction->operands[1].type ==
                                               OPERAND_REGISTER) ||
                                              (instruction->operands[1].type ==
                                               OPERAND_IMMEDIATE)) {
            try
              (jit_load_operand(bb, RBX, &instruction->operands[1])) try
                (get_pointer_to_data_region(bb, RDX)) try
                  (add_r64_r64(bb, RBX, RDX))
          }
        else {
          try
            (get_memory_operand_address(bb, RBX, &instruction->operands[1].mem))
        }
        try
          (jit_load_operand(bb, RCX, &instruction->operands[0])) try
            (bb_push(bb, PREFIX_16)) try
              (bb_push(bb, MOV_RM32_R32)) try
                (modrm_reg_indirect(bb, RBX, RCX)) break;
      case OP_STOREB:
        if ((instruction->operands[1].type == OPERAND_REGISTER) ||
            (instruction->operands[1].type == OPERAND_IMMEDIATE)) {
          try
            (jit_load_operand(bb, RBX, &instruction->operands[1])) try
              (get_pointer_to_data_region(bb, RDX)) try
                (add_r64_r64(bb, RBX, RDX))
        } else {
          try
            (get_memory_operand_address(bb, RBX, &instruction->operands[1].mem))
        }
        try
          (jit_load_operand(bb, RCX, &instruction->operands[0])) try
            (bb_push(bb, MOV_RM8_R8)) try
              (modrm_reg_indirect(bb, RBX, RCX)) try
                (set_register(bb, RCX, instruction->operands[0].reg)) break;
      case OP_LOAD:
        if ((instruction->operands[1].type == OPERAND_REGISTER) ||
            (instruction->operands[1].type == OPERAND_IMMEDIATE)) {
          try
            (jit_load_operand(bb, RBX, &instruction->operands[1])) try
              (get_pointer_to_data_region(bb, RDX)) try
                (add_r64_r64(bb, RBX, RDX))
        } else {
          try
            (get_memory_operand_address(bb, RBX, &instruction->operands[1].mem))
        }
        try
          (bb_push(bb, PREFIX_16)) try
            (bb_push(bb, MOV_R32_RM32)) try
              (modrm_reg_indirect(bb, RBX, RCX)) try
                (set_register(bb, RCX, instruction->operands[0].reg)) break;
      case OP_LOADB:
        if ((instruction->operands[1].type == OPERAND_REGISTER) ||
            (instruction->operands[1].type == OPERAND_IMMEDIATE)) {
          try
            (jit_load_operand(bb, RBX, &instruction->operands[1])) try
              (get_pointer_to_data_region(bb, RDX)) try
                (add_r64_r64(bb, RBX, RDX))
        } else {
          try
            (get_memory_operand_address(bb, RBX, &instruction->operands[1].mem))
        }
        try
          (bb_append(bb, MOVZX_R64_RM8, SIZEOF_MOVZX_R64_RM8)) try
            (modrm_reg_indirect(bb, RBX, RCX)) try
              (set_register(bb, RCX, instruction->operands[0].reg)) break;
      case OP_SYSCALL:
        try
          (get_flags(bb, RAX)) try
            (mov_r32_imm32(bb, RBX, FLAG_SYSCALL)) try
              (or_r64_r64(bb, RAX, RBX)) try
                (set_flags(bb, RAX)) break;
      }
  try
    (block_postamble(bb)) return SUCCESS;
}
int jit_set_block_accelerator(uint8_t *code, uint64_t address) {
  struct bb *bb = bb_create(128);
  try
    (mov_r64_imm64(bb, RAX, address)) try
      (bb_push(bb, PREFIX_REX)) try
        (bb_push(bb, MOV_RM32_R32)) try
          (modrm_reg_indirect_disp8(
              bb, RAX, RDI, offsetof(struct context, block_accelerator)))
              memcpy(code, bb_data(bb), bb_length(bb));
  bb_delete(bb);
  return 0;
}