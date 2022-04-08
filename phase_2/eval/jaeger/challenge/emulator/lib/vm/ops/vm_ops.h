#ifndef vm_ops_HEADER
#define vm_ops_HEADER

struct vm;
struct r5_instruction;

enum result vm_add(struct vm * vm, const struct r5_instruction * ins);
enum result vm_addi(struct vm * vm, const struct r5_instruction * ins);
enum result vm_and(struct vm * vm, const struct r5_instruction * ins);
enum result vm_andi(struct vm * vm, const struct r5_instruction * ins);
enum result vm_auipc(struct vm * vm, const struct r5_instruction * ins);
enum result vm_beq(struct vm * vm, const struct r5_instruction * ins);
enum result vm_bge(struct vm * vm, const struct r5_instruction * ins);
enum result vm_bgeu(struct vm * vm, const struct r5_instruction * ins);
enum result vm_blt(struct vm * vm, const struct r5_instruction * ins);
enum result vm_bltu(struct vm * vm, const struct r5_instruction * ins);
enum result vm_bne(struct vm * vm, const struct r5_instruction * ins);
enum result vm_div(struct vm * vm, const struct r5_instruction * ins);
enum result vm_divu(struct vm * vm, const struct r5_instruction * ins);
enum result vm_ebreak(struct vm * vm, const struct r5_instruction * ins);
enum result vm_jal(struct vm * vm, const struct r5_instruction * ins);
enum result vm_jalr(struct vm * vm, const struct r5_instruction * ins);
enum result vm_lb(struct vm * vm, const struct r5_instruction * ins);
enum result vm_lbu(struct vm * vm, const struct r5_instruction * ins);
enum result vm_lh(struct vm * vm, const struct r5_instruction * ins);
enum result vm_lhu(struct vm * vm, const struct r5_instruction * ins);
enum result vm_lui(struct vm * vm, const struct r5_instruction * ins);
enum result vm_lw(struct vm * vm, const struct r5_instruction * ins);
enum result vm_mret(struct vm * vm, const struct r5_instruction * ins);
enum result vm_mul(struct vm * vm, const struct r5_instruction * ins);
enum result vm_mulh(struct vm * vm, const struct r5_instruction * ins);
enum result vm_mulhu(struct vm * vm, const struct r5_instruction * ins);
enum result vm_mulhsu(struct vm * vm, const struct r5_instruction * ins);
enum result vm_or(struct vm * vm, const struct r5_instruction * ins);
enum result vm_ori(struct vm * vm, const struct r5_instruction * ins);
enum result vm_rem(struct vm * vm, const struct r5_instruction * ins);
enum result vm_remu(struct vm * vm, const struct r5_instruction * ins);
enum result vm_sb(struct vm * vm, const struct r5_instruction * ins);
enum result vm_sh(struct vm * vm, const struct r5_instruction * ins);
enum result vm_sw(struct vm * vm, const struct r5_instruction * ins);
enum result vm_sll(struct vm * vm, const struct r5_instruction * ins);
enum result vm_slli(struct vm * vm, const struct r5_instruction * ins);
enum result vm_slt(struct vm * vm, const struct r5_instruction * ins);
enum result vm_slti(struct vm * vm, const struct r5_instruction * ins);
enum result vm_sltu(struct vm * vm, const struct r5_instruction * ins);
enum result vm_sltiu(struct vm * vm, const struct r5_instruction * ins);
enum result vm_sra(struct vm * vm, const struct r5_instruction * ins);
enum result vm_srai(struct vm * vm, const struct r5_instruction * ins);
enum result vm_srl(struct vm * vm, const struct r5_instruction * ins);
enum result vm_srli(struct vm * vm, const struct r5_instruction * ins);
enum result vm_sub(struct vm * vm, const struct r5_instruction * ins);
enum result vm_wfi(struct vm * vm, const struct r5_instruction * ins);
enum result vm_xor(struct vm * vm, const struct r5_instruction * ins);
enum result vm_xori(struct vm * vm, const struct r5_instruction * ins);

#endif