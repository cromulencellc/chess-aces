#ifndef target_x86_HEADER
#define target_x86_HEADER
#include "container/bb.h"
#include <stdint.h>
#define RAX 0x0
#define RCX 0x1
#define RDX 0x2
#define RBX 0x3
#define RSP 0x4
#define RBP 0x5
#define RSI 0x6
#define RDI 0x7
#define CONTEXT_REG RDI
#define PREFIX_REX 0x48
#define PREFIX_16 0x66
#define ADD_RM32_R32 0x01
#define SUB_RM32_R32 0x29
#define AND_RM32_R32 0x21
#define OR_RM32_R32 0x09
#define XOR_RM32_R32 0x31
#define CMP_RM32_R32 0x39
#define MOV_RM8_R8 0x88
#define MOV_R8_RM8 0x8A
#define MOV_RM32_R32 0x89
#define MOV_RM32_IMM32 0xC7
#define MOV_R32_RM32 0x8B
extern const char MOVZX_R64_RM8[];
#define SIZEOF_MOVZX_R64_RM8 3
extern const char MOVZX_R64_RM16[];
#define SIZEOF_MOVZX_R64_RM16 3
int modrm_reg_indirect(struct bb *bb, uint8_t mr, uint8_t reg);
int modrm_reg_indirect_disp8(struct bb *bb, uint8_t reg, uint8_t mr,
                             int8_t disp8);
int modrm_reg_indirect_disp32(struct bb *bb, uint8_t mr, uint8_t reg,
                              int32_t disp32);
int mod_rm_reg_reg(struct bb *bb, uint8_t mr, uint8_t reg);
int mod_sib(struct bb *bb, uint8_t reg, uint8_t base, uint8_t index,
            uint8_t scale);
int mod_sib_disp8(struct bb *bb, uint8_t reg, uint8_t base, uint8_t index,
                  uint8_t scale, int8_t disp8);
int mod_sib_disp32(struct bb *bb, uint8_t reg, uint8_t base, uint8_t index,
                   uint8_t scale, int32_t disp32);
int pop_r(struct bb *bb, uint8_t reg);
int push_r(struct bb *bb, uint8_t reg);
int reg(struct bb *bb);
int add_r64_r64(struct bb *bb, uint8_t lhs, uint8_t rhs);
int add_r32_r32(struct bb *bb, uint8_t lhs, uint8_t rhs);
int add_r64_sext_imm8(struct bb *bb, uint8_t reg, int8_t imm8);
int add_r64_imm32(struct bb *bb, uint8_t reg, int32_t imm32);
int sub_r64_r64(struct bb *bb, uint8_t lhs, uint8_t rhs);
int mul_r64(struct bb *bb, uint8_t rhs);
int div_r64(struct bb *bb, uint8_t rhs);
int idiv_r64(struct bb *bb, uint8_t rhs);
int and_r64_r64(struct bb *bb, uint8_t lhs, uint8_t rhs);
int and_r64_sext_imm32(struct bb *bb, uint8_t lhs, uint32_t imm32);
int or_r64_r64(struct bb *bb, uint8_t lhs, uint8_t rhs);
int xor_r64_r64(struct bb *bb, uint8_t lhs, uint8_t rhs);
int shl_r64_cl(struct bb *bb, uint8_t r32);
int shr_r64_cl(struct bb *bb, uint8_t r32);
int asr_r64_cl(struct bb *bb, uint8_t r32);
int cmp_r64_r64(struct bb *bb, uint8_t lhs, uint8_t rhs);
int mov_r64_r64(struct bb *bb, uint8_t lhs, uint8_t rhs);
int mov_r32_imm32(struct bb *bb, uint8_t lhs, uint32_t imm32);
int mov_r64_imm64(struct bb *bb, uint8_t lhs, uint64_t imm64);
int block_preamble(struct bb *bb);
int block_postamble(struct bb *bb);
int cmp_flags_sequence(struct bb *bb, uint8_t lhs, uint8_t rhs, uint8_t dst);
int get_context(struct bb *bb, uint8_t reg);
int get_pc(struct bb *bb, uint8_t reg);
int set_pc(struct bb *bb, uint8_t reg);
int get_sp(struct bb *bb, uint8_t reg);
int set_sp(struct bb *bb, uint8_t reg);
int get_flags(struct bb *bb, uint8_t reg);
int set_flags(struct bb *bb, uint8_t reg);
int set_branch_address(struct bb *bb, uint8_t reg);
int get_memory_regions(struct bb *bb, uint8_t reg);
int set_block_accelerator(struct bb *bb, uint8_t reg);
int get_register(struct bb *bb, uint8_t x86_reg, uint8_t jb_reg);
int get_register_signed(struct bb *bb, uint8_t x86_reg, uint8_t jb_reg);
int set_register(struct bb *bb, uint8_t x86_reg, uint8_t jb_reg);
int test_cmp(const void *data0, const void *data1, uint32_t len);
int target_x86_test();
#endif