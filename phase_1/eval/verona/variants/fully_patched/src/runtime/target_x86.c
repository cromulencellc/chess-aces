#include "target_x86.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "container/bb.h"
#include "error.h"
#include "instructions.h"
#include "jit.h"
#include "rust.h"
const char MOVSX_R64_RM16[] = {PREFIX_REX, 0x0f, 0xbf};
const char MOVZX_R64_RM16[] = {PREFIX_REX, 0x0f, 0xb7};
const char MOVZX_R64_RM8[] = {PREFIX_REX, 0x0f, 0xb6};
int modrm_reg_indirect(struct bb *bb, uint8_t mr, uint8_t reg) {
  return bb_push(bb, mr | (reg << 3));
}
int modrm_reg_indirect_disp8(struct bb *bb, uint8_t reg, uint8_t mr,
                             int8_t disp8) {
  try
    (bb_push(bb, 0x40 | mr | (reg << 3)));
  try
    (bb_push(bb, (uint8_t)disp8));
  return 0;
}
int modrm_reg_indirect_disp32(struct bb *bb, uint8_t mr, uint8_t reg,
                              int32_t disp32) {
  try
    (bb_push(bb, 0x80 | mr | (reg << 3))) try
      (bb_push(bb, disp32 & 0xff)) try
        (bb_push(bb, (disp32 >> 8) & 0xff)) try
          (bb_push(bb, (disp32 >> 16) & 0xff)) try
            (bb_push(bb, (disp32 >> 24) & 0xff)) return 0;
}
int mod_rm_reg_reg(struct bb *bb, uint8_t mr, uint8_t reg) {
  try
    (bb_push(bb, 0xc0 | mr | (reg << 3))) return 0;
}
int mod_sib(struct bb *bb, uint8_t reg, uint8_t base, uint8_t index,
            uint8_t scale) {
  uint8_t scale_bits = 0;
  switch (scale) {
  case 0:
    break;
  case 2:
    scale_bits = 0x40;
    break;
  case 4:
    scale_bits = 0x80;
    break;
  case 8:
    scale_bits = 0xC0;
    break;
  default:
    return ERROR_INVALID_OPERANDS;
  }
  try
    (bb_push(bb, 0x04 | (reg << 3))) try
      (bb_push(bb, scale_bits | (index << 3) | base)) return 0;
}
int mod_sib_disp8(struct bb *bb, uint8_t reg, uint8_t base, uint8_t index,
                  uint8_t scale, int8_t disp8) {
  uint8_t scale_bits = 0;
  switch (scale) {
  case 0:
    break;
  case 2:
    scale_bits = 0x40;
    break;
  case 4:
    scale_bits = 0x80;
    break;
  case 8:
    scale_bits = 0xC0;
    break;
  default:
    return ERROR_INVALID_OPERANDS;
  }
  try
    (bb_push(bb, 0x44 | (reg << 3))) try
      (bb_push(bb, scale_bits | (index << 3) | base)) try
        (bb_push(bb, disp8)) return 0;
}
int mod_sib_disp32(struct bb *bb, uint8_t reg, uint8_t base, uint8_t index,
                   uint8_t scale, int32_t disp32) {
  uint8_t scale_bits = 0;
  switch (scale) {
  case 0:
    break;
  case 2:
    scale_bits = 0x40;
    break;
  case 4:
    scale_bits = 0x80;
    break;
  case 8:
    scale_bits = 0xC0;
    break;
  default:
    return ERROR_INVALID_OPERANDS;
  }
  try
    (bb_push(bb, 0x84 | (reg << 3))) try
      (bb_push(bb, scale_bits | (index << 3) | base)) try
        (bb_push(bb, disp32 & 0xff)) try
          (bb_push(bb, (disp32 >> 8) & 0xff)) try
            (bb_push(bb, (disp32 >> 16) & 0xff)) try
              (bb_push(bb, (disp32 >> 24) & 0xff)) return 0;
}
int pop_r(struct bb *bb, uint8_t reg) { return bb_push(bb, 0x58 | reg); }
int push_r(struct bb *bb, uint8_t reg) { return bb_push(bb, 0x50 | reg); }
int ret(struct bb *bb) { return bb_push(bb, 0xc3); }
int add_r64_r64(struct bb *bb, uint8_t lhs, uint8_t rhs) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, ADD_RM32_R32)) try
        (mod_rm_reg_reg(bb, lhs, rhs)) return 0;
}
int add_r32_r32(struct bb *bb, uint8_t lhs, uint8_t rhs) {
  try
    (bb_push(bb, ADD_RM32_R32)) try
      (mod_rm_reg_reg(bb, lhs, rhs)) return 0;
}
int add_r64_sext_imm8(struct bb *bb, uint8_t reg, int8_t imm8) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, 0x83)) try
        (bb_push(bb, 0xc0 | reg)) try
          (bb_push(bb, imm8)) return 0;
}
int add_r64_imm32(struct bb *bb, uint8_t reg, int32_t imm32) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, 0x81)) try
        (bb_push(bb, 0xc0 | reg)) try
          (bb_push(bb, imm32 & 0xff)) try
            (bb_push(bb, (imm32 >> 8) & 0xff)) try
              (bb_push(bb, (imm32 >> 16) & 0xff)) try
                (bb_push(bb, (imm32 >> 24) & 0xff)) return 0;
}
int sub_r64_r64(struct bb *bb, uint8_t lhs, uint8_t rhs) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, SUB_RM32_R32)) try
        (mod_rm_reg_reg(bb, lhs, rhs)) return 0;
}
int mul_r64(struct bb *bb, uint8_t rhs) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, 0xF7)) try
        (bb_push(bb, 0xE0 | rhs)) return 0;
}
int div_r64(struct bb *bb, uint8_t rhs) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, 0xF7)) try
        (bb_push(bb, 0xF0 | rhs)) return 0;
}
int idiv_r64(struct bb *bb, uint8_t rhs) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, 0xF7)) try
        (bb_push(bb, 0xF8 | rhs)) return 0;
}
int and_r64_sext_imm32(struct bb *bb, uint8_t lhs, uint32_t imm32) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, 0x81)) try
        (bb_push(bb, 0xe0 | lhs)) try
          (bb_push(bb, imm32 & 0xff)) try
            (bb_push(bb, (imm32 >> 8) & 0xff)) try
              (bb_push(bb, (imm32 >> 16) & 0xff)) try
                (bb_push(bb, (imm32 >> 24) & 0xff)) return 0;
}
int and_r64_r64(struct bb *bb, uint8_t lhs, uint8_t rhs) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, AND_RM32_R32)) try
        (mod_rm_reg_reg(bb, lhs, rhs)) return 0;
}
int or_r64_r64(struct bb *bb, uint8_t lhs, uint8_t rhs) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, OR_RM32_R32)) try
        (mod_rm_reg_reg(bb, lhs, rhs)) return 0;
}
int xor_r64_r64(struct bb *bb, uint8_t lhs, uint8_t rhs) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, XOR_RM32_R32)) try
        (mod_rm_reg_reg(bb, lhs, rhs)) return 0;
}
int shl_r64_cl(struct bb *bb, uint8_t r32) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, 0xd3)) try
        (bb_push(bb, 0xe0 | r32)) return 0;
}
int shr_r64_cl(struct bb *bb, uint8_t r32) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, 0xd3)) try
        (bb_push(bb, 0xe8 | r32)) return 0;
}
int asr_r64_cl(struct bb *bb, uint8_t r32) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, 0xd3)) try
        (bb_push(bb, 0xf8 | r32)) return 0;
}
int cmp_r64_r64(struct bb *bb, uint8_t lhs, uint8_t rhs) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, CMP_RM32_R32)) try
        (mod_rm_reg_reg(bb, lhs, rhs)) return 0;
}
int mov_r32_r32(struct bb *bb, uint8_t lhs, uint8_t rhs) {
  try
    (bb_push(bb, MOV_RM32_R32)) try
      (mod_rm_reg_reg(bb, lhs, rhs)) return 0;
}
int mov_r64_r64(struct bb *bb, uint8_t lhs, uint8_t rhs) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, MOV_RM32_R32)) try
        (mod_rm_reg_reg(bb, lhs, rhs)) return 0;
}
int mov_r32_imm32(struct bb *bb, uint8_t lhs, uint32_t imm32) {
  try
    (bb_push(bb, 0xb8 | lhs)) try
      (bb_push(bb, imm32 & 0xff)) try
        (bb_push(bb, (imm32 >> 8) & 0xff)) try
          (bb_push(bb, (imm32 >> 16) & 0xff)) try
            (bb_push(bb, (imm32 >> 24) & 0xff)) return 0;
}
int mov_r64_imm64(struct bb *bb, uint8_t lhs, uint64_t imm64) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, 0xb8 | lhs)) try
        (bb_push(bb, imm64 & 0xff)) try
          (bb_push(bb, (imm64 >> 8) & 0xff)) try
            (bb_push(bb, (imm64 >> 16) & 0xff)) try
              (bb_push(bb, (imm64 >> 24) & 0xff)) try
                (bb_push(bb, (imm64 >> 32) & 0xff)) try
                  (bb_push(bb, (imm64 >> 40) & 0xff)) try
                    (bb_push(bb, (imm64 >> 48) & 0xff)) try
                      (bb_push(bb, (imm64 >> 56) & 0xff)) return 0;
}
int cmp_flags_sequence(struct bb *bb, uint8_t lhs, uint8_t rhs, uint8_t dst) {
  try
    (xor_r64_r64(bb, dst, dst)) try
      (cmp_r64_r64(bb, lhs, rhs)) try
        (bb_push(bb, 0x75)) try
          (bb_push(bb, 0x06)) try
            (bb_push(bb, 0x81)) try
              (bb_push(bb, 0xc0 | dst)) try
                (bb_push(bb, FLAG_EQUAL & 0xff)) try
                  (bb_push(bb, (FLAG_EQUAL >> 8) & 0xff)) try
                    (bb_push(bb, (FLAG_EQUAL >> 16) & 0xff)) try
                      (bb_push(bb, (FLAG_EQUAL >> 24) & 0xff)) try
                        (bb_push(bb, 0x7f)) try
                          (bb_push(bb, 0x06)) try
                            (bb_push(bb, 0x81)) try
                              (bb_push(bb, 0xc0 | dst)) try
                                (bb_push(bb, FLAG_LESS_THAN_SIGNED & 0xff)) try
                                  (bb_push(bb, (FLAG_LESS_THAN_SIGNED >> 8) &
                                                   0xff)) try
                                    (bb_push(bb, (FLAG_LESS_THAN_SIGNED >> 16) &
                                                     0xff)) try
                                      (bb_push(bb,
                                               (FLAG_LESS_THAN_SIGNED >> 24) &
                                                   0xff)) try
                                        (bb_push(bb, 0x77)) try
                                          (bb_push(bb, 0x06)) try
                                            (bb_push(bb, 0x81)) try
                                              (bb_push(bb, 0xc0 | dst)) try
                                                (bb_push(
                                                    bb,
                                                    FLAG_LESS_THAN_UNSIGNED &
                                                        0xff)) try
                                                  (bb_push(
                                                      bb,
                                                      (FLAG_LESS_THAN_UNSIGNED >>
                                                       8) &
                                                          0xff)) try
                                                    (bb_push(
                                                        bb,
                                                        (FLAG_LESS_THAN_UNSIGNED >>
                                                         16) &
                                                            0xff)) try
                                                      (bb_push(
                                                          bb,
                                                          (FLAG_LESS_THAN_UNSIGNED >>
                                                           24) &
                                                              0xff)) return 0;
}
int get_context(struct bb *bb, uint8_t reg) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, MOV_R32_RM32)) try
        (modrm_reg_indirect_disp8(bb, reg, RBP, -8)) return 0;
}
int block_preamble(struct bb *bb) {
  try
    (push_r(bb, RBP)) try
      (mov_r64_r64(bb, RBP, RSP)) try
        (push_r(bb, RDI)) try
          (push_r(bb, RBX)) try
            (get_context(bb, CONTEXT_REG)) return 0;
}
int block_postamble(struct bb *bb) {
  try
    (xor_r64_r64(bb, RAX, RAX)) try
      (pop_r(bb, RDI)) try
        (pop_r(bb, RBX)) try
          (pop_r(bb, RBP)) try
            (bb_push(bb, 0xc3)) return 0;
}
int get_pc(struct bb *bb, uint8_t reg) {
  try
    (bb_append(bb, MOVZX_R64_RM16, sizeof(MOVZX_R64_RM16))) try
      (modrm_reg_indirect_disp8(bb, reg, CONTEXT_REG,
                                offsetof(struct context, pc))) return 0;
}
int set_pc(struct bb *bb, uint8_t reg) {
  try
    (bb_push(bb, PREFIX_16)) try
      (bb_push(bb, MOV_RM32_R32)) try
        (modrm_reg_indirect_disp8(bb, reg, CONTEXT_REG,
                                  offsetof(struct context, pc))) return 0;
}
int get_sp(struct bb *bb, uint8_t reg) {
  try
    (bb_append(bb, MOVZX_R64_RM16, sizeof(MOVZX_R64_RM16))) try
      (modrm_reg_indirect_disp8(bb, reg, CONTEXT_REG,
                                offsetof(struct context, gprs) +
                                    (SP * 2))) return 0;
}
int set_sp(struct bb *bb, uint8_t reg) {
  try
    (bb_push(bb, PREFIX_16)) try
      (bb_push(bb, MOV_RM32_R32)) try
        (modrm_reg_indirect_disp8(bb, reg, CONTEXT_REG,
                                  offsetof(struct context, gprs) +
                                      (SP * 2))) return 0;
}
int get_flags(struct bb *bb, uint8_t reg) {
  try
    (bb_push(bb, PREFIX_16)) try
      (bb_push(bb, MOV_R32_RM32)) try
        (modrm_reg_indirect_disp8(bb, reg, CONTEXT_REG,
                                  offsetof(struct context, flags))) return 0;
}
int set_flags(struct bb *bb, uint8_t reg) {
  try
    (bb_push(bb, PREFIX_16)) try
      (bb_push(bb, MOV_RM32_R32)) try
        (modrm_reg_indirect_disp8(bb, reg, CONTEXT_REG,
                                  offsetof(struct context, flags))) return 0;
}
int set_branch_address(struct bb *bb, uint8_t reg) {
  try
    (bb_push(bb, PREFIX_16)) try
      (bb_push(bb, MOV_RM32_R32)) try
        (modrm_reg_indirect_disp8(
            bb, reg, CONTEXT_REG,
            offsetof(struct context, branch_address))) return 0;
}
int get_memory_regions(struct bb *bb, uint8_t reg) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, MOV_R32_RM32)) try
        (modrm_reg_indirect_disp8(bb, reg, CONTEXT_REG,
                                  offsetof(struct context, memory))) return 0;
}
int set_block_accelerator(struct bb *bb, uint8_t reg) {
  try
    (bb_push(bb, PREFIX_REX)) try
      (bb_push(bb, MOV_RM32_R32)) try
        (modrm_reg_indirect_disp8(
            bb, reg, CONTEXT_REG,
            offsetof(struct context, block_accelerator))) return 0;
}
int get_register(struct bb *bb, uint8_t x86_reg, uint8_t jb_reg) {
  try
    (bb_append(bb, MOVZX_R64_RM16, sizeof(MOVZX_R64_RM16))) int8_t offset =
        offsetof(struct context, gprs);
  offset += (jb_reg * 2);
  try
    (modrm_reg_indirect_disp8(bb, x86_reg, CONTEXT_REG, offset)) return 0;
}
int get_register_signed(struct bb *bb, uint8_t x86_reg, uint8_t jb_reg) {
  try
    (bb_append(bb, MOVSX_R64_RM16, sizeof(MOVSX_R64_RM16))) int8_t offset =
        offsetof(struct context, gprs);
  offset += (jb_reg * 2);
  try
    (modrm_reg_indirect_disp8(bb, x86_reg, CONTEXT_REG, offset)) return 0;
}
int set_register(struct bb *bb, uint8_t x86_reg, uint8_t jb_reg) {
  try
    (bb_push(bb, PREFIX_16)) try
      (bb_push(bb, MOV_RM32_R32)) int8_t offset =
          offsetof(struct context, gprs);
  offset += (jb_reg * 2);
  try
    (modrm_reg_indirect_disp8(bb, x86_reg, CONTEXT_REG, offset)) return 0;
}