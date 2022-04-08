#include "cpu.hpp"
#include "syscall_int.hpp"
#include <cassert>
#define MSB(value, bits) ((value >> (bits - 1)) & 1)
void opcount_check(cs_insn *insn, int expected) {
  cs_x86 *x86;
  assert(insn != NULL);
  x86 = &(insn->detail->x86);
  if (x86->op_count != expected) {
    std::cout << "[FAIL] opcount mismatch: expected: " << (int)expected
              << " received: " << (int)x86->op_count << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
  }
  return;
}
int Cpu::can_exec(uint64_t addr) {
  MemBlock *m = mem->getBlock(addr);
  if (m == NULL) {
    return 0;
  }
  if (!(mem->getPerms(addr) & PROT_EXEC)) {
    return 0;
  }
  return 1;
}
uint64_t SE(uint64_t value, uint64_t bits) {
  uint64_t sign_bit = 0;
  if (bits >= 64) {
    return value;
  }
  sign_bit = MSB(value, bits);
  if (sign_bit == 0) {
    return value;
  }
  for (int i = bits; i < 64; i++) {
    value |= (sign_bit << i);
  }
  return value;
}
uint256_t SE(uint256_t value, uint64_t bits) {
  uint64_t sign_bit = 0;
  uint256_t temp = value;
  if (bits >= 256) {
    return temp;
  }
  sign_bit = MSB(value, bits);
  if (sign_bit == 0) {
    return temp;
  }
  for (uint256_t i = bits; i < 256; i++) {
    value |= (sign_bit << i);
  }
  return value;
}
int sign_extend(uint64_t in, uint64_t *out, uint64_t bits) {
  uint64_t value = 0;
  uint64_t sign_bit;
  if (out == NULL) {
    printf("outnul\n");
    return -1;
  }
  if (bits > 64) {
    printf("bad bits\n");
    return -1;
  }
  sign_bit = (in >> (bits - 1)) & 1;
  value = in;
  if (sign_bit == 0) {
    *out = value;
    return 0;
  }
  for (int i = bits; i < 64; i++) {
    value |= (sign_bit << i);
  }
  *out = value;
  return 0;
}
int sign_extend(uint256_t in, uint256_t *out, uint64_t bits) {
  uint256_t value = 0;
  uint256_t sign_bit;
  if (out == NULL) {
    return -1;
  }
  if (bits > 256) {
    return -1;
  }
  sign_bit = MSB(in, bits);
  value = in;
  if (sign_bit == 0) {
    *out = value;
    return 0;
  }
  for (int i = bits; i < 256; i++) {
    value |= (sign_bit << i);
  }
  *out = value;
  return 0;
}
int setpf(uint64_t a) {
  int p = 0;
  for (int i = 0; i < 8; i++) {
    if ((a >> i) & 1UL) {
      p++;
    }
  }
  if (p % 2) {
    return 0;
  }
  return 1;
}
int setcf_sub(uint64_t a, uint64_t b, uint64_t bits) {
  uint64_t ta = a;
  uint64_t tb = b;
  uint64_t borrow_bit = 0;
  uint8_t bita = 0;
  uint8_t bitb = 0;
  for (int i = 0; i < bits; i++) {
    bita = (ta >> i) & 1;
    bitb = (tb >> i) & 1;
    if (bita == 0 && bitb == 1) {
      borrow_bit = i + 1;
      while (!((ta >> borrow_bit) & 1) && borrow_bit < bits) {
        borrow_bit++;
      }
      if (borrow_bit == bits) {
        return 1;
      }
      ta ^= (1 << borrow_bit);
      for (int j = i + 1; j < borrow_bit; j++) {
        ta ^= (1 << j);
      }
    }
  }
  return 0;
}
int setaf_sub(uint64_t a, uint64_t b) { return setcf_sub(a, b, 4); }
int Cpu::read_mem_operand(cs_x86_op *readop, uint64_t *value) {
  uint64_t tvalue = 0;
  uint64_t read_addr = 0;
  uint64_t base_reg_value = 0;
  uint64_t index = 0;
  uint64_t displacement = 0;
  uint64_t scale = 1;
  if (readop == NULL || value == NULL) {
    return -1;
  }
  if (readop->mem.base != X86_REG_INVALID) {
    base_reg_value = getreg(cs_reg_name(handle, readop->mem.base));
  }
  if (readop->mem.index != X86_REG_INVALID) {
    index = getreg(cs_reg_name(handle, readop->mem.index));
  }
  scale = readop->mem.scale;
  if (readop->mem.disp != 0) {
    displacement = readop->mem.disp;
  }
  read_addr = base_reg_value + displacement;
  if (index) {
    read_addr += (index * scale);
  }
  if (readop->mem.segment != X86_REG_INVALID) {
    if (read_segment_mem(cs_reg_name(handle, readop->mem.segment), read_addr,
                         (char *)&tvalue, (uint64_t)readop->size) == -1) {
      std::cout << "[ERROR] segfault attempted read of segment offset 0x"
                << std::hex << read_addr << std::endl;
      exit(0);
    }
  } else {
    if (mem->readint((void *)read_addr, &tvalue, (uint64_t)readop->size) ==
        -1) {
      std::cout << "[ERROR] segfault attempted read of 0x" << std::hex
                << read_addr << std::endl;
      exit(0);
    }
  }
  *value = tvalue;
  return 0;
}
int Cpu::read_mem_operand(cs_x86_op *readop, uint256_t *value) {
  uint256_t tvalue = 0;
  uint64_t read_addr = 0;
  uint64_t base_reg_value = 0;
  uint64_t index = 0;
  uint64_t displacement = 0;
  uint64_t scale = 1;
  if (readop == NULL || value == NULL) {
    return -1;
  }
  if (readop->mem.base != X86_REG_INVALID) {
    base_reg_value = getreg(cs_reg_name(handle, readop->mem.base));
  }
  if (readop->mem.index != X86_REG_INVALID) {
    index = getreg(cs_reg_name(handle, readop->mem.index));
  }
  scale = readop->mem.scale;
  if (readop->mem.disp != 0) {
    displacement = readop->mem.disp;
  }
  read_addr = base_reg_value + displacement;
  if (index) {
    read_addr += (index * scale);
  }
  if (readop->mem.segment != X86_REG_INVALID) {
    std::cout << "[ERROR] segment register with 256" << std::endl;
    exit(0);
  } else {
    if (mem->readint((void *)read_addr, &tvalue, (uint64_t)readop->size) ==
        -1) {
      std::cout << "[ERROR] segfault attempted read of 0x" << std::hex
                << read_addr << std::endl;
      exit(0);
    }
  }
  *value = tvalue;
  return 0;
}
int Cpu::write_mem_operand(cs_x86_op *writeop, uint64_t value) {
  uint64_t write_addr = 0;
  uint64_t base_reg_value = 0;
  uint64_t index = 0;
  uint64_t displacement = 0;
  uint64_t scale = 0;
  if (writeop == NULL) {
    return -1;
  }
  if (writeop->mem.base != X86_REG_INVALID) {
    base_reg_value = getreg(cs_reg_name(handle, writeop->mem.base));
  }
  if (writeop->mem.index != X86_REG_INVALID) {
    index = getreg(cs_reg_name(handle, writeop->mem.index));
  }
  scale = writeop->mem.scale;
  if (writeop->mem.disp != 0) {
    displacement = writeop->mem.disp;
  }
  write_addr = base_reg_value + displacement;
  if (index) {
    write_addr += (index * scale);
  }
  if (writeop->mem.segment != X86_REG_INVALID) {
    if (write_segment_mem(cs_reg_name(handle, writeop->mem.segment), write_addr,
                          (char *)&value, writeop->size) == -1) {
      std::cout << "[ERROR] segfault attempted write of segment offset 0x"
                << std::hex << write_addr << std::endl;
      exit(0);
    }
  } else if (mem->write_l((void *)write_addr, (const char *)&value,
                          writeop->size) != 0) {
    std::cout << "[ERROR] segfault writing to: 0x" << write_addr << std::endl;
    exit(0);
  }
  return 0;
}
int Cpu::write_mem_operand(cs_x86_op *writeop, uint256_t value) {
  uint64_t write_addr = 0;
  uint64_t base_reg_value = 0;
  uint64_t index = 0;
  uint64_t displacement = 0;
  uint64_t scale = 0;
  char data[128];
  assert(writeop != NULL);
  if (writeop->mem.base != X86_REG_INVALID) {
    base_reg_value = getreg(cs_reg_name(handle, writeop->mem.base));
  }
  if (writeop->mem.index != X86_REG_INVALID) {
    index = getreg(cs_reg_name(handle, writeop->mem.index));
  }
  scale = writeop->mem.scale;
  if (writeop->mem.disp != 0) {
    displacement = writeop->mem.disp;
  }
  write_addr = base_reg_value + displacement;
  if (index) {
    write_addr += (index * scale);
  }
  if (writeop->mem.segment != X86_REG_INVALID) {
    memset(data, 0, 128);
    for (int i = 0; i < writeop->size && i < 128; i++) {
      data[i] = (uint8_t)(value & 0xff);
      value >>= 8;
    }
    if (write_segment_mem(cs_reg_name(handle, writeop->mem.segment), write_addr,
                          data, writeop->size) == -1) {
      std::cout << "[ERROR] segfault attempted write of segment offset 0x"
                << std::hex << write_addr << std::endl;
      exit(0);
    }
  } else if (mem->writeymm((void *)write_addr, value, writeop->size) != 0) {
    std::cout << "[ERROR] segfault writing to: 0x" << write_addr << std::endl;
    exit(0);
  }
  return 0;
}
int Cpu::mov(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    value = readop->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &value);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    if (x86->prefix[2] == 0x66) {
      setreg(cs_reg_name(handle, writeop->reg), value);
    } else {
      if (writeop->size == 4) {
        setreg(mapToReg64(cs_reg_name(handle, writeop->reg)), value);
      } else {
        setreg(cs_reg_name(handle, writeop->reg), value);
      }
    }
    break;
  case X86_OP_MEM:
    write_mem_operand(writeop, value);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 1;
}
int Cpu::call(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *calldest;
  uint64_t call_addr = 0;
  assert(insn != NULL);
  _rip += insn->size;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 1);
  calldest = &(x86->operands[0]);
  switch ((int)calldest->type) {
  case X86_OP_REG:
    call_addr = getreg(cs_reg_name(handle, calldest->reg));
    break;
  case X86_OP_IMM:
    call_addr = calldest->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(calldest, &call_addr);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)calldest->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  uint64_t next_rip = _rip.val;
  _rsp -= sizeof(void *);
  if (mem->write_l((void *)_rsp.val, (const char *)&next_rip, sizeof(void *)) !=
      0) {
    std::cout << "[ERROR] segfault writing to: 0x" << _rsp.val << std::endl;
    exit(0);
  }
  _rip.val = call_addr;
  return 1;
}
int Cpu::push(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *writeop;
  uint64_t write_value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  writeop = &(x86->operands[0]);
  switch ((int)writeop->type) {
  case X86_OP_REG:
    write_value = getreg(cs_reg_name(handle, writeop->reg));
    break;
  case X86_OP_IMM:
    write_value = writeop->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(writeop, &write_value);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  _rsp -= writeop->size;
  if (mem->write_l((void *)_rsp.val, (const char *)&write_value,
                   writeop->size) != 0) {
    std::cout << "[ERROR] segfault writing to: 0x" << _rsp.val << std::endl;
    exit(0);
  }
  return 1;
}
int Cpu::sub(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t value = 0;
  uint64_t subvalue = 0;
  uint64_t result = 0;
  uint8_t sign_value = 0;
  uint8_t sign_subvalue = 0;
  uint8_t sign_result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    subvalue = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    subvalue = readop->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &subvalue);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  subvalue = SE(subvalue, readop->size * 8);
  switch ((int)writeop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, writeop->reg));
    value = SE(value, writeop->size * 8);
    result = value - subvalue;
    setreg(cs_reg_name(handle, writeop->reg), result);
    break;
  case X86_OP_MEM:
    read_mem_operand(writeop, &value);
    value = SE(value, writeop->size * 8);
    result = value - subvalue;
    write_mem_operand(writeop, result);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sign_value = MSB(value, readop->size * 8);
  sign_subvalue = MSB(subvalue, writeop->size * 8);
  sign_result = MSB(result, writeop->size * 8);
  of = (sign_value ^ sign_subvalue) & (sign_value ^ sign_result);
  sf = sign_result;
  zf = (result == 0);
  cf = value < subvalue;
  pf = setpf(result);
  af = ((value ^ subvalue ^ result) & 0x10) != 0;
  return 1;
}
int Cpu::rdtsc(cs_insn *insn) {
  setreg("rax", rdtsc_value & 0xffffffff);
  setreg("rdx", rdtsc_value >> 32);
  _rip += insn->size;
  return rdtsc_value;
}
int Cpu::shl(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t bitcount = 0;
  uint64_t value = 0;
  uint64_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    bitcount = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    bitcount = readop->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &bitcount);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (x86->rex == 0) {
    bitcount = bitcount & 0x1f;
  } else {
    bitcount = bitcount & 0x3f;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, writeop->reg));
    result = value << bitcount;
    if (writeop->size != 8) {
      result = result & ((1ULL << writeop->size * 8) - 1);
    }
    setreg(cs_reg_name(handle, writeop->reg), result);
    break;
  case X86_OP_MEM:
    read_mem_operand(writeop, &value);
    result = value << bitcount;
    result = result & ((1ULL << writeop->size * 8) - 1);
    write_mem_operand(writeop, result);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  uint64_t temp = value;
  uint64_t temp_shift = (writeop->size * 8) - bitcount;
  if (bitcount == 0) {
    return 1;
  }
  cf = (temp >> temp_shift) & 1;
  of = ((result ^ value) >> ((writeop->size * 8) - 1)) & 1;
  sf = (result >> ((writeop->size * 8) - 1)) & 1;
  zf = (result == 0);
  pf = setpf(result);
  af = 0;
  return 1;
}
int Cpu::or_ins(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t dest = 0;
  uint64_t source = 0;
  uint64_t result = 0;
  assert(insn != NULL);
  _rip += insn->size;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    source = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    source = readop->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &source);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    dest = getreg(cs_reg_name(handle, writeop->reg));
    result = dest | source;
    setreg(cs_reg_name(handle, writeop->reg), result);
    break;
  case X86_OP_MEM:
    read_mem_operand(writeop, &dest);
    result = dest | source;
    write_mem_operand(writeop, result);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  of = 0;
  cf = 0;
  sf = (result >> ((writeop->size * 8) - 1)) & 1;
  zf = (result == 0);
  pf = setpf(result);
  af = 0;
  return 1;
}
int Cpu::lea(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t base_reg_value = 0;
  uint64_t index = 0;
  uint64_t scale = 0;
  uint64_t displacement = 0;
  uint64_t read_addr = 0;
  assert(insn != NULL);
  _rip += insn->size;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_MEM:
    if (readop->mem.base != X86_REG_INVALID) {
      base_reg_value = getreg(cs_reg_name(handle, readop->mem.base));
    }
    if (readop->mem.index != X86_REG_INVALID) {
      index = getreg(cs_reg_name(handle, readop->mem.index));
    }
    scale = readop->mem.scale;
    if (readop->mem.disp != 0) {
      displacement = readop->mem.disp;
    }
    read_addr = base_reg_value + displacement;
    if (index) {
      read_addr += (index * scale);
    }
    break;
  default:
    std::cout << "[ERROR] Unhandled readop operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, writeop->reg), read_addr);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 1;
}
int Cpu::test(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t value = 0;
  uint64_t b = 0;
  uint64_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    value = readop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    b = getreg(cs_reg_name(handle, writeop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(writeop, &b);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  result = value & b;
  of = 0;
  cf = 0;
  sf = (result >> ((writeop->size * 8) - 1)) & 1;
  zf = (result == 0);
  pf = setpf(result);
  return 1;
}
int Cpu::je(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_IMM:
    value = destop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (zf == 1) {
    _rip.val = value;
  }
  return 0;
}
int Cpu::add(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t value = 0;
  uint64_t addvalue = 0;
  uint64_t result = 0;
  uint8_t sign_value = 0;
  uint8_t sign_addvalue = 0;
  uint8_t sign_result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    addvalue = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    addvalue = readop->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &addvalue);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  addvalue = SE(addvalue, readop->size * 8);
  switch ((int)writeop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, writeop->reg));
    value = SE(value, writeop->size * 8);
    result = value + addvalue;
    setreg(cs_reg_name(handle, writeop->reg), result);
    break;
  case X86_OP_MEM:
    read_mem_operand(writeop, &value);
    value = SE(value, writeop->size * 8);
    result = value + addvalue;
    write_mem_operand(writeop, result);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sign_value = MSB(value, readop->size * 8);
  sign_addvalue = MSB(addvalue, writeop->size * 8);
  sign_result = MSB(result, writeop->size * 8);
  of = (sign_value ^ sign_addvalue ^ 1) & (sign_value ^ sign_result);
  sf = sign_result;
  zf = (result == 0);
  cf = (result < value) | (result < addvalue);
  pf = setpf(result);
  af = ((value ^ addvalue ^ result) & 0x10) != 0;
  return 1;
}
int Cpu::jmp(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, destop->reg));
    break;
  case X86_OP_IMM:
    value = destop->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(destop, &value);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  _rip.val = value;
  return 0;
}
int Cpu::cmp(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t value = 0;
  uint64_t subvalue = 0;
  uint64_t result = 0;
  uint8_t sign_value = 0;
  uint8_t sign_subvalue = 0;
  uint8_t sign_result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    subvalue = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    subvalue = readop->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &subvalue);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  subvalue = SE(subvalue, readop->size * 8);
  switch ((int)writeop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, writeop->reg));
    value = SE(value, writeop->size * 8);
    result = value - subvalue;
    break;
  case X86_OP_MEM:
    read_mem_operand(writeop, &value);
    value = SE(value, writeop->size * 8);
    result = value - subvalue;
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sign_value = MSB(value, readop->size * 8);
  sign_subvalue = MSB(subvalue, writeop->size * 8);
  sign_result = MSB(result, writeop->size * 8);
  of = (sign_value ^ sign_subvalue) & (sign_value ^ sign_result);
  sf = sign_result;
  zf = (result == 0);
  cf = value < subvalue;
  pf = setpf(result);
  af = ((value ^ subvalue ^ result) & 0x10) != 0;
  return 1;
}
int Cpu::jbe(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_IMM:
    value = destop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (zf == 1 || cf == 1) {
    _rip.val = value;
  }
  return 0;
}
int Cpu::sar(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t bitcount = 0;
  uint64_t value = 0;
  uint64_t result = 0;
  uint64_t sign_bit = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    bitcount = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    bitcount = readop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (x86->rex != 0) {
    bitcount &= 0x3f;
  } else {
    bitcount &= 0x1f;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, writeop->reg));
    sign_bit = MSB(value, writeop->size * 8);
    for (int i = 0; i < bitcount; i++) {
      cf = value & 1;
      value >>= 1;
      value |= sign_bit << ((writeop->size * 8) - 1);
    }
    result = value;
    if (x86->prefix[2] == 0x66) {
      setreg(cs_reg_name(handle, writeop->reg), result);
    } else {
      if (writeop->size == 4) {
        setreg(mapToReg64(cs_reg_name(handle, writeop->reg)), result);
      } else {
        setreg(cs_reg_name(handle, writeop->reg), result);
      }
    }
    break;
  case X86_OP_MEM:
    read_mem_operand(writeop, &value);
    sign_bit = MSB(value, writeop->size * 8);
    for (int i = 0; i < bitcount; i++) {
      cf = value & 1;
      value >>= 1;
      value |= sign_bit << ((writeop->size * 8) - 1);
    }
    result = value;
    write_mem_operand(writeop, result);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (bitcount == 0) {
    return 1;
  }
  if (bitcount == 1) {
    of = 0;
  }
  sf = MSB(result, writeop->size * 8);
  zf = (result == 0);
  pf = setpf(result);
  af = 0;
  return 1;
}
int Cpu::ja(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_IMM:
    value = destop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (zf == 0 && cf == 0) {
    _rip.val = value;
  }
  return 0;
}
int Cpu::pop(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *writeop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  writeop = &(x86->operands[0]);
  switch ((int)writeop->type) {
  case X86_OP_REG:
    if (mem->readint((void *)_rsp.val, &value, (uint64_t)writeop->size) == -1) {
      std::cout << "[ERROR] segfault attempted read of 0x" << std::hex
                << _rsp.val << std::endl;
      exit(0);
    }
    setreg(cs_reg_name(handle, writeop->reg), value);
    break;
  case X86_OP_MEM:
    if (mem->readint((void *)_rsp.val, &value, (uint64_t)writeop->size) == -1) {
      std::cout << "[ERROR] segfault attempted read of 0x" << std::hex
                << _rsp.val << std::endl;
      exit(0);
    }
    write_mem_operand(writeop, value);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  _rsp += writeop->size;
  return 1;
}
int Cpu::xor_ins(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t dest = 0;
  uint64_t source = 0;
  uint64_t result = 0;
  assert(insn != NULL);
  _rip += insn->size;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    source = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    source = readop->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &source);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    dest = getreg(cs_reg_name(handle, writeop->reg));
    result = dest ^ source;
    setreg(cs_reg_name(handle, writeop->reg), result);
    break;
  case X86_OP_MEM:
    read_mem_operand(writeop, &dest);
    result = dest ^ source;
    write_mem_operand(writeop, result);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  of = 0;
  cf = 0;
  sf = MSB(result, writeop->size * 8);
  zf = (result == 0);
  pf = setpf(result);
  af = 0;
  return 1;
}
int Cpu::and_ins(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t dest = 0;
  uint64_t source = 0;
  uint64_t result = 0;
  assert(insn != NULL);
  _rip += insn->size;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    source = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    source = readop->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &source);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    dest = getreg(cs_reg_name(handle, writeop->reg));
    result = dest & source;
    setreg(cs_reg_name(handle, writeop->reg), result);
    break;
  case X86_OP_MEM:
    read_mem_operand(writeop, &dest);
    result = dest & source;
    write_mem_operand(writeop, result);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  of = 0;
  cf = 0;
  sf = (result >> ((writeop->size * 8) - 1)) & 1;
  zf = (result == 0);
  pf = setpf(result);
  af = 0;
  return 1;
}
int Cpu::ret(cs_insn *insn) {
  uint64_t value = 0;
  assert(insn != NULL);
  _rip += insn->size;
  if (mem->readint((void *)_rsp.val, &value, sizeof(void *)) == -1) {
    std::cout << "[ERROR] segfault attempted read of 0x" << std::hex << _rsp.val
              << std::endl;
    exit(0);
  }
  _rsp += sizeof(void *);
  _rip.val = value;
  return 1;
}
int Cpu::movsxd(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    value = readop->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &value);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sign_extend(value, &value, readop->size * 8);
  switch ((int)writeop->type) {
  case X86_OP_REG:
    if (writeop->size != 8) {
      value = value & ((1ULL << (writeop->size * 8)) - 1);
    }
    setreg(cs_reg_name(handle, writeop->reg), value);
    break;
  case X86_OP_MEM:
    if (writeop->size != 8) {
      value = value & ((1ULL << (writeop->size * 8)) - 1);
    }
    write_mem_operand(writeop, value);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 1;
}
int Cpu::jne(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_IMM:
    value = destop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (zf == 0) {
    _rip.val = value;
  }
  return 0;
}
int Cpu::movzx(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    value = readop->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &value);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    if (x86->prefix[2] == 0x66) {
      setreg(cs_reg_name(handle, writeop->reg), value);
    } else {
      setreg(mapToReg64(cs_reg_name(handle, writeop->reg)), value);
    }
    break;
  case X86_OP_MEM:
    write_mem_operand(writeop, value);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 1;
}
int Cpu::nop(cs_insn *insn) {
  _rip += insn->size;
  return 0;
}
int Cpu::cmovne(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t value = 0;
  assert(insn != NULL);
  _rip += insn->size;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &value);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    if (zf != 0) {
      value = getreg(cs_reg_name(handle, writeop->reg));
    }
    if (x86->prefix[2] == 0x66) {
      setreg(cs_reg_name(handle, writeop->reg), value);
    } else {
      if (writeop->size == 4) {
        setreg(mapToReg64(cs_reg_name(handle, writeop->reg)), value);
      } else {
        setreg(cs_reg_name(handle, writeop->reg), value);
      }
    }
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 1;
}
int Cpu::cpuid(cs_insn *insn) {
  uint64_t value;
  uint64_t trcx;
  if (!insn) {
    return -1;
  }
  _rip += insn->size;
  value = getreg("rax");
  switch (value) {
  case 0:
    setreg("rax", 0x14);
    setreg("rbx", 0x756e6547);
    setreg("rcx", 0x6c65746e);
    setreg("rdx", 0x49656e69);
    break;
  case 2:
    setreg("rax", 0x76036301);
    setreg("rbx", 0xf0b5ff);
    setreg("rcx", 0);
    setreg("rdx", 0xc30000);
  case 1:
    setreg("rax", 0x306d4);
    setreg("rbx", 0x20800);
    setreg("rcx", 0xdefa2203);
    setreg("rdx", 0x178bfbff);
    break;
  case 4:
    trcx = getreg("ecx");
    if (trcx == 0) {
      setreg("rax", 0x4000121);
      setreg("rbx", 0x1c0003f);
      setreg("rcx", 0x3f);
      setreg("rdx", 0);
    } else if (trcx == 1) {
      setreg("rax", 0x4000122);
      setreg("rbx", 0x1c0003f);
      setreg("rcx", 0x3f);
      setreg("rdx", 0);
    } else if (trcx == 2) {
      setreg("rax", 0x4000143);
      setreg("rbx", 0x1c0003f);
      setreg("rcx", 0x1ff);
      setreg("rdx", 0);
    } else if (trcx == 3) {
      setreg("rax", 0x4000163);
      setreg("rbx", 0x3c0003f);
      setreg("rcx", 0xfff);
      setreg("rdx", 6);
    } else {
      std::cout << "[ERROR] Unhandled cpuid rax = 0x4, rcx = 0x" << trcx
                << std::endl;
      exit(0);
    }
    break;
  case 7:
    setreg("rax", 0);
    setreg("rbx", 0x42421);
    setreg("rcx", 0);
    setreg("rdx", 0x10000400);
    break;
  case 0xd:
    trcx = getreg("ecx");
    if (trcx == 0) {
      setreg("rax", 7);
      setreg("rbx", 0x340);
      setreg("rcx", 0x340);
      setreg("rdx", 0);
    } else if (trcx == 1) {
      setreg("rax", 0);
      setreg("rbx", 0);
      setreg("rcx", 0);
      setreg("rdx", 0);
    } else {
      std::cout << "[ERROR] Unhandled cpuid rax = 0xd, rcx = 0x" << trcx
                << std::endl;
      exit(0);
    }
    break;
  default:
    std::cout << "[ERROR] Unhandled cpuid value: " << value << std::endl;
    exit(0);
    break;
  };
  return 0;
}
int Cpu::shr(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t bitcount = 0;
  uint64_t value = 0;
  uint64_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    bitcount = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    bitcount = readop->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &bitcount);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (x86->rex != 0) {
    bitcount &= 0x3f;
  } else {
    bitcount &= 0x1f;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, writeop->reg));
    result = value >> bitcount;
    if (x86->prefix[2] == 0x66) {
      setreg(cs_reg_name(handle, writeop->reg), result);
    } else {
      if (writeop->size == 4) {
        setreg(mapToReg64(cs_reg_name(handle, writeop->reg)), result);
      } else {
        setreg(cs_reg_name(handle, writeop->reg), result);
      }
    }
    break;
  case X86_OP_MEM:
    read_mem_operand(writeop, &value);
    result = value >> bitcount;
    write_mem_operand(writeop, result);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (bitcount == 0) {
    return 1;
  }
  cf = (value >> (bitcount - 1)) & 1;
  of = ((value >> ((writeop->size * 8) - 1)) & 1);
  sf = (result >> ((writeop->size * 8) - 1)) & 1;
  zf = (result == 0);
  pf = setpf(result);
  af = 0;
  return 1;
}
int Cpu::jle(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_IMM:
    value = destop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (zf == 1 || (sf != of)) {
    _rip.val = value;
  }
  return 0;
}
int Cpu::xgetbv(cs_insn *insn) {
  _rip += insn->size;
  if (getreg("rcx") != 0) {
    std::cout << "[ERROR] Only handling rcx == 0" << std::endl;
    exit(0);
  }
  setreg("rax", 7);
  setreg("rdx", 0);
  return 0;
}
int Cpu::bt(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t dest = 0;
  uint64_t source = 0;
  assert(insn != NULL);
  _rip += insn->size;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    source = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    source = readop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  source %= (writeop->size * 8);
  switch ((int)writeop->type) {
  case X86_OP_REG:
    dest = getreg(cs_reg_name(handle, writeop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(writeop, &dest);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  dest >>= source;
  cf = dest & 1;
  return 1;
}
int Cpu::jb(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_IMM:
    value = destop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (cf == 1) {
    _rip.val = value;
  }
  return 0;
}
int Cpu::jae(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_IMM:
    value = destop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (cf == 0) {
    _rip.val = value;
  }
  return 0;
}
int Cpu::cmove(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t value = 0;
  assert(insn != NULL);
  _rip += insn->size;
  if (zf != 1) {
    return 0;
  }
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &value);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, writeop->reg), value);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 1;
}
int Cpu::syscall(cs_insn *insn) {
  _rip += insn->size;
  setreg("rcx", _rip.val);
  throw SycallException();
  return 0;
}
int Cpu::js(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_IMM:
    value = destop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (sf == 1) {
    _rip.val = value;
  }
  return 0;
}
int Cpu::setb(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t b;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  b = cf;
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), b);
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, b);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 0;
}
int Cpu::div_ins(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *denom_op;
  uint256_t numerator = 0;
  uint256_t result = 0;
  uint64_t denominator = 0;
  uint64_t lower = 0;
  uint64_t upper = 0;
  uint64_t rem = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  denom_op = &(x86->operands[0]);
  switch ((int)denom_op->type) {
  case X86_OP_REG:
    denominator = getreg(cs_reg_name(handle, denom_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(denom_op, &denominator);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)denom_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch (denom_op->size) {
  case 1:
    lower = getreg("ax");
    upper = 0;
    break;
  case 2:
    lower = getreg("ax");
    upper = getreg("dx");
    break;
  case 4:
    lower = getreg("eax");
    upper = getreg("edx");
    break;
  case 8:
    lower = getreg("rax");
    upper = getreg("rdx");
    break;
  default:
    std::cout << "[ERROR] Unhandled size: " << (int)denom_op->size << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
  };
  numerator = uint256_t(lower) | (uint256_t(upper) << (denom_op->size * 8));
  if (denominator == 0) {
    std::cout << "[EXCEPTION] divide by zero" << std::endl;
    exit(0);
  }
  result = numerator / denominator;
  rem = numerator % denominator;
  if (result >> (denom_op->size) * 8) {
    std::cout << "[EXCEPTION] Arithmetic exception" << std::endl;
    exit(0);
  }
  switch (denom_op->size) {
  case 1:
    setreg("al", (uint64_t)(result & 0xff));
    setreg("ah", (uint64_t)(rem & 0xff));
    break;
  case 2:
    setreg("ax", (uint64_t)(result & 0xffff));
    setreg("dx", (uint64_t)(rem & 0xffff));
    break;
  case 4:
    setreg("eax", (uint64_t)(result & 0xffffffff));
    setreg("edx", (uint64_t)(rem & 0xffffffff));
    break;
  case 8:
    setreg("rax", (uint64_t)(result & 0xffffffffffffffff));
    setreg("rdx", (uint64_t)(rem & 0xffffffffffffffff));
    break;
  default:
    std::cout << "[ERROR] Unhandled size: " << (int)denom_op->size << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
  };
  return 1;
}
int Cpu::imul_single(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *opa;
  uint256_t multa = 0;
  uint256_t multb = 0;
  uint256_t result = 0;
  uint64_t value = 0;
  uint256_t sign_checka = 0;
  uint256_t sign_checkb = 0;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opa = &(x86->operands[0]);
  switch ((int)opa->type) {
  case X86_OP_REG:
    multa = uint256_t(getreg(cs_reg_name(handle, opa->reg)));
    break;
  case X86_OP_MEM:
    read_mem_operand(opa, &multa);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)opa->type
              << std::endl;
    exit(0);
    break;
  }
  multb = uint256_t(getreg(cs_reg_name(handle, detail->regs_read[0])));
  sign_extend(multa, &multa, opa->size * 8);
  sign_extend(multb, &multb, opa->size * 8);
  result = multa * multb;
  if (opa->size == 1) {
    setreg("ax", (uint64_t)(result & 0xffff));
    sign_checka = (uint256_t(1) << (opa->size * 8)) - 1;
    sign_checka = result & sign_checka;
    sign_checkb = (uint256_t(1) << (opa->size * 16)) - 1;
    sign_checkb = result & sign_checkb;
    sign_extend(sign_checka, &sign_checka, opa->size * 8);
    sign_extend(sign_checkb, &sign_checkb, opa->size * 16);
    if (sign_checkb == sign_checka) {
      cf = 0;
      of = 0;
    } else {
      cf = 1;
      of = 1;
    }
    if ((result >> 15) & 1) {
      sf = 1;
    } else {
      sf = 0;
    }
    zf = 0;
    pf = setpf(result);
  } else {
    if (opa->size == 8) {
      value = result;
    } else {
      value = (1ULL << (opa->size * 8)) - 1UL;
      value = result & value;
    }
    if (x86->prefix[2] == 0x66) {
      setreg(cs_reg_name(handle, detail->regs_write[0]), value);
    } else {
      if (opa->size == 4) {
        setreg(mapToReg64(cs_reg_name(handle, detail->regs_write[0])), value);
      } else {
        setreg(cs_reg_name(handle, detail->regs_write[0]), value);
      }
    }
    sign_checka = (uint256_t(1) << (opa->size * 8)) - 1;
    sign_checka = result & sign_checka;
    sign_checkb = (uint256_t(1) << (opa->size * 16)) - 1;
    sign_checkb = result & sign_checkb;
    sign_extend(sign_checka, &sign_checka, opa->size * 8);
    sign_extend(sign_checkb, &sign_checkb, opa->size * 16);
    if (sign_checkb == sign_checka) {
      cf = 0;
      of = 0;
    } else {
      cf = 1;
      of = 1;
    }
    if ((result >> ((opa->size * 8) - 1)) & 1) {
      sf = 1;
    } else {
      sf = 0;
    }
    zf = 0;
    pf = setpf(result);
    result >>= (opa->size * 8);
    setreg(cs_reg_name(handle, detail->regs_write[1]), (uint64_t)result);
  }
  return 1;
}
int Cpu::imul_double(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *opa, *opb;
  uint256_t multa = 0;
  uint256_t multb = 0;
  uint256_t result = 0;
  uint256_t sign_checka = 0;
  uint256_t sign_checkb = 0;
  uint64_t value = 0;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opa = &(x86->operands[0]);
  opb = &(x86->operands[1]);
  switch ((int)opa->type) {
  case X86_OP_REG:
    multa = uint256_t(getreg(cs_reg_name(handle, opa->reg)));
    break;
  case X86_OP_MEM:
    read_mem_operand(opa, &multa);
    break;
  default:
    std::cout << "[ERROR] Unhandled operand type: " << (int)opa->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)opb->type) {
  case X86_OP_REG:
    multb = uint256_t(getreg(cs_reg_name(handle, opb->reg)));
    break;
  case X86_OP_MEM:
    read_mem_operand(opb, &multb);
    break;
  default:
    std::cout << "[ERROR] Unhandled operand type: " << (int)opa->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sign_extend(multa, &multa, opa->size * 8);
  sign_extend(multb, &multb, opa->size * 8);
  result = multa * multb;
  sign_checka = (uint256_t(1) << (opa->size * 8)) - 1;
  sign_checka = result & sign_checka;
  sign_checkb = (uint256_t(1) << (opa->size * 16)) - 1;
  sign_checkb = result & sign_checkb;
  sign_extend(sign_checka, &sign_checka, opa->size * 8);
  sign_extend(sign_checkb, &sign_checkb, opa->size * 16);
  if (sign_checkb == sign_checka) {
    cf = 0;
    of = 0;
  } else {
    cf = 1;
    of = 1;
  }
  if ((result >> ((opa->size * 8) - 1)) & 1) {
    sf = 1;
  } else {
    sf = 0;
  }
  zf = 0;
  pf = setpf(result);
  switch ((int)opa->type) {
  case X86_OP_REG:
    if (opa->size == 8) {
      value = result;
    } else {
      value = (1UL << (opa->size * 8)) - 1UL;
      value = result & value;
    }
    if (x86->prefix[2] == 0x66 || opa->size != 4) {
      setreg(cs_reg_name(handle, opa->reg), value);
    } else {
      setreg(mapToReg64(cs_reg_name(handle, opa->reg)), value);
    }
    break;
  default:
    std::cout << "[ERROR] Unhandled operand type: " << (int)opa->type
              << std::endl;
    exit(0);
    break;
  }
  return 1;
}
int Cpu::imul_triple(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *opa, *opb, *opc;
  uint256_t multa = 0;
  uint256_t multb = 0;
  uint256_t result = 0;
  uint64_t value = 0;
  uint256_t sign_checka = 0;
  uint256_t sign_checkb = 0;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opa = &(x86->operands[0]);
  opb = &(x86->operands[1]);
  opc = &(x86->operands[2]);
  switch ((int)opc->type) {
  case X86_OP_IMM:
    multa = uint256_t(opc->imm);
    break;
  default:
    std::cout << "[ERROR] Unhandled operand type: " << (int)opc->type
              << std::endl;
    exit(0);
  }
  switch ((int)opb->type) {
  case X86_OP_REG:
    multb = uint256_t(getreg(cs_reg_name(handle, opb->reg)));
    break;
  case X86_OP_MEM:
    read_mem_operand(opb, &multb);
    break;
  default:
    std::cout << "[ERROR] Unhandled operand atype: " << (int)opb->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sign_extend(multa, &multa, opa->size * 8);
  sign_extend(multb, &multb, opa->size * 8);
  result = multa * multb;
  sign_checka = (uint256_t(1) << (opa->size * 8)) - 1;
  sign_checka = result & sign_checka;
  sign_checkb = (uint256_t(1) << (opa->size * 16)) - 1;
  sign_checkb = result & sign_checkb;
  sign_extend(sign_checka, &sign_checka, opa->size * 8);
  sign_extend(sign_checkb, &sign_checkb, opa->size * 16);
  if (sign_checkb == sign_checka) {
    cf = 0;
    of = 0;
  } else {
    cf = 1;
    of = 1;
  }
  if ((result >> ((opa->size * 8) - 1)) & 1) {
    sf = 1;
  } else {
    sf = 0;
  }
  zf = 0;
  pf = setpf(result);
  switch ((int)opa->type) {
  case X86_OP_REG:
    if (opa->size == 8) {
      value = result;
    } else {
      value = (1ULL << (opa->size * 8)) - 1ULL;
      value = result & value;
    }
    if (x86->prefix[2] == 0x66 || opa->size != 4) {
      setreg(cs_reg_name(handle, opa->reg), value);
    } else {
      setreg(mapToReg64(cs_reg_name(handle, opa->reg)), value);
    }
    break;
  default:
    std::cout << "[ERROR] Unhandled operand type: " << (int)opa->type
              << std::endl;
    exit(0);
  }
  return 1;
}
int Cpu::imul(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  if (x86->op_count == 1) {
    return imul_single(insn);
  } else if (x86->op_count == 2) {
    return imul_double(insn);
  } else if (x86->op_count == 3) {
    return imul_triple(insn);
  } else {
    std::cout << "[ERROR] Invalid op count " << (int)x86->op_count << std::endl;
    exit(0);
  }
  return 0;
}
int Cpu::vpxor(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op_a, *src_op_b;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 3);
  destop = &(x86->operands[0]);
  src_op_a = &(x86->operands[1]);
  src_op_b = &(x86->operands[2]);
  switch ((int)src_op_a->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op_a->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op_a->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)src_op_b->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, src_op_b->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op_b, &sourceb);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand b: " << (int)src_op_b->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  result = sourcea ^ sourceb;
  if (destop->size == 16) {
    result &= ((uint256_t(1) << 128) - 1);
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    if (destop->size == 16) {
      setreg(mapToRegStruct[cs_reg_name(handle, destop->reg)]->parent, result);
    }
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::vpcmpeqb(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op_a, *src_op_b;
  uint8_t ba = 0;
  uint8_t bb = 0;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 3);
  destop = &(x86->operands[0]);
  src_op_a = &(x86->operands[1]);
  src_op_b = &(x86->operands[2]);
  switch ((int)src_op_a->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op_a->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op_a->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)src_op_b->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, src_op_b->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op_b, &sourceb);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand b: " << (int)src_op_b->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  for (int i = 0; i < src_op_b->size; i++) {
    ba = sourcea & 0xff;
    bb = sourceb & 0xff;
    if (ba == bb) {
      result |= (uint256_t(0xff) << (i * 8));
    }
    sourcea >>= 8;
    sourceb >>= 8;
  }
  if (destop->size == 16) {
    result &= ((uint256_t(1) << 128) - 1);
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    if (destop->size == 16) {
      setreg(mapToRegStruct[cs_reg_name(handle, destop->reg)]->parent, result);
    }
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::vpmovmskb(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  uint8_t t;
  uint64_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  for (int i = 0; i < src_op->size; i++) {
    t = sourcea & 0xff;
    sourcea >>= 8;
    t = (t >> 7) & 1;
    result |= ((uint64_t)t) << i;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::tzcnt(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t dest = 0;
  uint64_t source = 0;
  assert(insn != NULL);
  _rip += insn->size;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    source = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &source);
    break;
  default:
    std::cout << "[ERROR] Unhandled read operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  for (int i = 0; i < readop->size * 8; i++) {
    if ((source & 1) == 0) {
      dest++;
    } else {
      break;
    }
    source >>= 1UL;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, writeop->reg), dest);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (dest == readop->size * 8) {
    cf = 1;
  } else {
    cf = 0;
  }
  if (dest == 0) {
    zf = 1;
  } else {
    zf = 0;
  }
  return 1;
}
int Cpu::vzeroupper(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  uint256_t temp = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 0);
  for (int i = 0; i < detail->regs_write_count; i++) {
    temp = getymm(cs_reg_name(handle, detail->regs_write[i]));
    temp = (temp << uint256_t(128)) >> (uint256_t(128));
    setreg(cs_reg_name(handle, detail->regs_write[i]), temp);
  }
  printExtendedState();
  return 1;
}
int Cpu::pxor(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, destop->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  result = sourcea ^ sourceb;
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::movdqa(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), sourcea);
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::pcmpeqb(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint8_t ba = 0;
  uint8_t bb = 0;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, destop->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  for (int i = 0; i < src_op->size; i++) {
    ba = sourcea & 0xff;
    bb = sourceb & 0xff;
    if (ba == bb) {
      result |= (uint256_t(0xff) << (i * 8));
    }
    sourcea >>= 8;
    sourceb >>= 8;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::pcmpeqd(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint32_t ba = 0;
  uint32_t bb = 0;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, destop->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  for (int i = 0; i < src_op->size / 4; i++) {
    ba = sourcea & 0xffffffff;
    bb = sourceb & 0xffffffff;
    if (ba == bb) {
      result |= (uint256_t(0xffffffff) << (i * 32));
    }
    sourcea >>= 32;
    sourceb >>= 32;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::pslldq(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, destop->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)src_op->type) {
  case X86_OP_IMM:
    sourcea = uint256_t(src_op->imm);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (sourcea > 15) {
    result = 0;
  } else {
    result = sourceb << (sourcea * 8);
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::psubb(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint8_t ba = 0;
  uint8_t bb = 0;
  uint8_t bc = 0;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, destop->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  for (int i = 0; i < src_op->size; i++) {
    ba = sourcea & 0xff;
    bb = sourceb & 0xff;
    bc = bb - ba;
    result |= (uint256_t(bc) << (i * 8));
    sourcea >>= 8;
    sourceb >>= 8;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::movss(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    sourceb = getymm(cs_reg_name(handle, destop->reg));
    value = ((sourceb >> 32) << 32) | (sourcea & 0xffffffff);
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &value);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), value);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::pmovmskb(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  uint8_t t;
  uint64_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  for (int i = 0; i < src_op->size; i++) {
    t = sourcea & 0xff;
    sourcea >>= 8;
    t = (t >> 7) & 1;
    result |= ((uint64_t)t) << i;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::bsf(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t dest = 0;
  uint64_t source = 0;
  assert(insn != NULL);
  _rip += insn->size;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    source = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &source);
    break;
  default:
    std::cout << "[ERROR] Unhandled read operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  pf = setpf(source);
  cf = 0;
  if (source == 0) {
    zf = 1;
    dest = readop->size * 8;
  } else {
    zf = 0;
    for (int i = 0; i < readop->size * 8; i++) {
      if (source & 1) {
        break;
      }
      source >>= 1;
      dest++;
    }
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, writeop->reg), dest);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 1;
}
int Cpu::movsx(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t value = 0;
  uint64_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    value = readop->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &value);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sign_extend(value, &result, readop->size * 8);
  if (writeop->size != 8) {
    result &= ((1ULL << (writeop->size * 8)) - 1);
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    if (x86->prefix[2] == 0x66 || writeop->size != 4) {
      setreg(cs_reg_name(handle, writeop->reg), result);
    } else {
      setreg(mapToReg64(cs_reg_name(handle, writeop->reg)), result);
    }
    break;
  case X86_OP_MEM:
    write_mem_operand(writeop, result);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 1;
}
int Cpu::jg(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_IMM:
    value = destop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (zf == 0 && sf == of) {
    _rip.val = value;
  }
  return 0;
}
int Cpu::vmovdqu(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    if (src_op->size == 16) {
      sourcea &= ((uint256_t(1) << 128) - 1);
      setreg(mapToRegStruct[cs_reg_name(handle, destop->reg)]->parent, sourcea);
    } else {
      setreg(cs_reg_name(handle, destop->reg), sourcea);
    }
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::movq(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    if (src_op->size == 8) {
      sourcea = uint256_t(getreg(cs_reg_name(handle, src_op->reg)));
    } else if (src_op->size == 16) {
      sourcea = getymm(cs_reg_name(handle, src_op->reg));
    } else {
      std::cout << "[ERROR] unexpected size" << std::endl;
      exit(0);
    }
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), sourcea);
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::punpcklqdq(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, destop->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  result =
      ((sourcea & 0xffffffffffffffff) << 64) | (sourceb & 0xffffffffffffffff);
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::paddq(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t temp_op = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, destop->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  temp_op = ((uint64_t)(sourcea & 0xffffffffffffffff)) +
            ((uint64_t)(sourceb & 0xffffffffffffffff));
  result = temp_op;
  sourcea >>= 64;
  sourceb >>= 64;
  temp_op = ((uint64_t)(sourcea & 0xffffffffffffffff)) +
            ((uint64_t)(sourceb & 0xffffffffffffffff));
  result |= temp_op << 64;
  ;
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::paddd(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t temp_op = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, destop->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  for (int i = 0; i < destop->size / 4; i++) {
    temp_op =
        ((uint32_t)(sourcea & 0xffffffff)) + ((uint32_t)(sourceb & 0xffffffff));
    result |= (temp_op << i * 32);
    sourcea >>= 32;
    sourceb >>= 32;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::pslld(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t temp_op = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  case X86_OP_IMM:
    sourcea = src_op->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, destop->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  for (int i = 0; i < destop->size / 4; i++) {
    temp_op = sourceb << sourcea;
    temp_op &= 0xffffffff;
    result |= (temp_op << i * 32);
    sourceb >>= 32;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::pcmpgtd(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  int32_t ba = 0;
  int32_t bb = 0;
  uint32_t temp = 0;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, destop->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  for (int i = 0; i < src_op->size / 4; i++) {
    temp = (uint32_t)(sourcea & 0xffffffff);
    ba = (int32_t)temp;
    temp = (uint32_t)(sourceb & 0xffffffff);
    bb = (int32_t)temp;
    if (bb > ba) {
      result |= (uint256_t(0xffffffff) << (i * 32));
    }
    sourcea >>= 32;
    sourceb >>= 32;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::punpckldq(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t templow = 0;
  uint256_t temphigh = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, destop->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  for (int i = 0; i < src_op->size / 8; i++) {
    templow = (sourceb & 0xffffffff);
    temphigh = (sourcea & 0xffffffff);
    result |= ((temphigh << 32) | templow) << (i * 64);
    sourcea >>= 32;
    sourceb >>= 32;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::punpckhdq(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t templow = 0;
  uint256_t temphigh = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, destop->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sourcea >>= 64;
  sourceb >>= 64;
  for (int i = 0; i < src_op->size / 8; i++) {
    templow = (sourceb & 0xffffffff);
    temphigh = (sourcea & 0xffffffff);
    result |= ((temphigh << 32) | templow) << (i * 64);
    sourcea >>= 32;
    sourceb >>= 32;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::psllq(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t temp_op = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  case X86_OP_IMM:
    sourcea = src_op->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, destop->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  for (int i = 0; i < destop->size / 8; i++) {
    temp_op = sourceb << sourcea;
    temp_op &= 0xffffffffffffffff;
    result |= (temp_op << i * 64);
    sourceb >>= 64;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::punpckhqdq(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t templow = 0;
  uint256_t temphigh = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, destop->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sourcea >>= 64;
  sourceb >>= 64;
  for (int i = 0; i < src_op->size / 16; i++) {
    templow = (sourceb & 0xffffffffffffffff);
    temphigh = (sourcea & 0xffffffffffffffff);
    result |= ((temphigh << 64) | templow) << (i * 128);
    sourcea >>= 64;
    sourceb >>= 64;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::movaps(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), sourcea);
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::seta(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t b;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  if (cf == 0 && zf == 0) {
    b = 1;
  } else {
    b = 0;
  }
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), b);
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, b);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 0;
}
int Cpu::cmpxchg(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *srcop;
  uint64_t raxval = 0;
  uint64_t srcval = 0;
  uint64_t destval = 0;
  int64_t rt = 0;
  int64_t st = 0;
  int64_t dt = 0;
  int64_t result = 0;
  uint8_t sign_value = 0;
  uint8_t sign_subvalue = 0;
  uint8_t sign_result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  srcop = &(x86->operands[1]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    destval = getreg(cs_reg_name(handle, destop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(destop, &destval);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sign_extend(destval, &destval, destop->size * 8);
  dt = (int64_t)destval;
  switch ((int)srcop->type) {
  case X86_OP_REG:
    srcval = getreg(cs_reg_name(handle, srcop->reg));
    sign_extend(srcval, &srcval, srcop->size * 8);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)srcop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  st = (int64_t)srcval;
  switch (destop->size) {
  case 1:
    raxval = getreg("al");
    break;
  case 2:
    raxval = getreg("ax");
    break;
  case 4:
    raxval = getreg("eax");
    break;
  case 8:
    raxval = getreg("rax");
    break;
  default:
    std::cout << "[ERROR] unhandled size: " << (int)destop->size << std::endl;
    exit(0);
  };
  sign_extend(raxval, &raxval, destop->size * 8);
  rt = (int64_t)raxval;
  result = rt - dt;
  if (result == 0) {
    zf = 1;
    switch ((int)destop->type) {
    case X86_OP_REG:
      setreg(cs_reg_name(handle, destop->reg), srcval);
      break;
    case X86_OP_MEM:
      write_mem_operand(destop, srcval);
      break;
    default:
      std::cout << "[ERROR] Unhandled write operand: " << (int)destop->type
                << std::endl;
      printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
             insn->op_str, insn->id);
      exit(0);
      break;
    }
  } else {
    zf = 0;
    switch (destop->size) {
    case 1:
      setreg("al", destval);
      break;
    case 2:
      setreg("ax", destval);
      break;
    case 4:
      setreg("eax", destval);
      break;
    case 8:
      setreg("rax", destval);
      break;
    };
  }
  sign_value = (rt >> ((destop->size * 8) - 1)) & 1;
  sign_subvalue = (dt >> ((destop->size * 8) - 1)) & 1;
  sign_result = (result >> ((destop->size * 8) - 1)) & 1;
  of = (sign_value ^ sign_subvalue) & (sign_value ^ sign_result);
  sf = sign_result;
  cf = raxval < destval;
  pf = setpf(result);
  af = (((dt ^ rt) ^ result) & 0x10) != 0;
  return 1;
}
int Cpu::neg(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  uint64_t destval = 0;
  uint64_t result = 0;
  cs_x86_op *destop;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    destval = getreg(cs_reg_name(handle, destop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(destop, &destval);
    break;
  default:
    std::cout << "[ERROR] Unhandled read operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sign_extend(destval, &destval, destop->size * 8);
  result = 0 - destval;
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, result);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (result == 0) {
    cf = 0;
  } else {
    cf = 1;
  }
  of = (result == (1ULL << ((destop->size * 8) - 1)));
  sf = (result >> ((destop->size * 8) - 1)) & 1;
  af = ((result & 0x0f) != 0);
  zf = (result == 0);
  pf = setpf(result);
  return 1;
}
int Cpu::movups(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), sourcea);
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::dec(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  uint64_t destval = 0;
  uint64_t result = 0;
  cs_x86_op *destop;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    destval = getreg(cs_reg_name(handle, destop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(destop, &destval);
    break;
  default:
    std::cout << "[ERROR] Unhandled read operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sign_extend(destval, &destval, destop->size * 8);
  result = destval - 1;
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, result);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  of = (result == ~(1ULL << ((destop->size * 8) - 1)));
  sf = (result >> ((destop->size * 8) - 1)) & 1;
  af = ((result & 0x0f) == 0x0f);
  zf = (result == 0);
  pf = setpf(result);
  return 1;
}
int Cpu::cmova(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t value = 0;
  assert(insn != NULL);
  _rip += insn->size;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &value);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    if ((zf != 0) || (cf != 0)) {
      value = getreg(cs_reg_name(handle, writeop->reg));
    }
    if (x86->prefix[2] == 0x66) {
      setreg(cs_reg_name(handle, writeop->reg), value);
    } else {
      if (writeop->size == 4) {
        setreg(mapToReg64(cs_reg_name(handle, writeop->reg)), value);
      } else {
        setreg(cs_reg_name(handle, writeop->reg), value);
      }
    }
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 1;
}
int Cpu::setne(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t b;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  b = (zf == 0);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), b);
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, b);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 0;
}
int Cpu::stosq(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t write_data = 0;
  uint64_t counter = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 2);
  if (x86->prefix[0] == 0) {
    _rip += insn->size;
  } else {
    counter = getreg(cs_reg_name(handle, detail->regs_read[1]));
    if (counter == 0) {
      _rip += insn->size;
      return 0;
    }
    counter -= 1;
    if (counter == 0) {
      _rip += insn->size;
    }
    setreg(cs_reg_name(handle, detail->regs_read[1]), counter);
  }
  write_data = getreg(cs_reg_name(handle, detail->regs_read[0]));
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_MEM:
    write_mem_operand(destop, write_data);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  counter = getreg(cs_reg_name(handle, detail->regs_read[2]));
  if (df == 1) {
    counter -= (int)destop->size;
  } else {
    counter += (int)destop->size;
  }
  setreg(cs_reg_name(handle, detail->regs_read[2]), counter);
  return 0;
}
int Cpu::clc(cs_insn *insn) {
  _rip += insn->size;
  cf = 0;
  return 0;
}
int Cpu::stc(cs_insn *insn) {
  _rip += insn->size;
  cf = 1;
  return 0;
}
int Cpu::sbb(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t value = 0;
  uint64_t subvalue = 0;
  uint64_t result = 0;
  uint8_t sign_value = 0;
  uint8_t sign_subvalue = 0;
  uint8_t sign_result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    subvalue = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    subvalue = readop->imm;
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &subvalue);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sign_extend(subvalue, &subvalue, readop->size * 8);
  subvalue += cf;
  switch ((int)writeop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, writeop->reg));
    sign_extend(value, &value, writeop->size * 8);
    result = value - subvalue;
    setreg(cs_reg_name(handle, writeop->reg), result);
    break;
  case X86_OP_MEM:
    read_mem_operand(writeop, &value);
    sign_extend(value, &value, writeop->size * 8);
    result = value - subvalue;
    write_mem_operand(writeop, result);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sign_value = (value >> ((readop->size * 8) - 1)) & 1;
  sign_subvalue = (subvalue >> ((writeop->size * 8) - 1)) & 1;
  sign_result = (result >> ((writeop->size * 8) - 1)) & 1;
  of = (sign_value ^ sign_subvalue) & (sign_value ^ sign_result);
  sf = sign_result;
  zf = (result == 0);
  cf = setcf_sub(value, subvalue, writeop->size * 8);
  pf = setpf(result);
  af = setaf_sub(value, subvalue);
  return 1;
}
int Cpu::cdqe(cs_insn *insn) {
  cs_detail *detail;
  rtype *rax_t = NULL;
  uint64_t value = 0;
  uint64_t bits = 0;
  assert(insn != NULL);
  detail = insn->detail;
  _rip += insn->size;
  rax_t = mapToRegStruct[cs_reg_name(handle, detail->regs_read[0])];
  bits = rax_t->bits;
  value = getreg(cs_reg_name(handle, detail->regs_read[0]));
  sign_extend(value, &value, bits);
  setreg(cs_reg_name(handle, detail->regs_write[0]), value);
  return 1;
}
int Cpu::mul(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  uint64_t destval = 0;
  uint64_t raxval = 0;
  uint256_t result = 0;
  uint256_t a = 0;
  uint256_t b = 0;
  cs_x86_op *destop;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    destval = getreg(cs_reg_name(handle, destop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(destop, &destval);
    break;
  default:
    std::cout << "[ERROR] Unhandled read operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  raxval = getreg(cs_reg_name(handle, detail->regs_read[0]));
  a = uint256_t(raxval);
  b = uint256_t(destval);
  result = a * b;
  pf = setpf((uint64_t)(result));
  sf = (result >> ((destop->size * 8) - 1)) & 1;
  if (detail->regs_read[0] == X86_REG_AL) {
    setreg("ax", (uint64_t)result);
  } else {
    setreg(cs_reg_name(handle, detail->regs_read[0]), (uint64_t)result);
  }
  result >>= destop->size * 8;
  if (result == 0) {
    of = 0;
    cf = 0;
  } else {
    of = 1;
    cf = 1;
  }
  switch (detail->regs_read[0]) {
  case X86_REG_AL:
    break;
  case X86_REG_AX:
    setreg("dx", ((uint64_t)result));
    break;
  case X86_REG_EAX:
    setreg("edx", ((uint64_t)result));
    break;
  case X86_REG_RAX:
    setreg("rdx", ((uint64_t)result));
    break;
  default:
    std::cout << "[ERROR] unknown reg" << std::endl;
    exit(0);
    break;
  };
  return 1;
}
int Cpu::vmovd(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t td = 0;
  uint64_t v = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    if (src_op->size < 16) {
      td = uint256_t(getreg(cs_reg_name(handle, src_op->reg)));
    } else {
      td = getymm(cs_reg_name(handle, src_op->reg));
    }
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &td);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    if (destop->size == 16) {
      td &= ((uint256_t(1) << 128) - 1);
      setreg(mapToRegStruct[cs_reg_name(handle, destop->reg)]->parent, td);
    } else if (destop->size == 4) {
      v = (uint64_t)td;
      setreg(cs_reg_name(handle, destop->reg), v);
    } else {
      std::cout << "[ERROR] Unhandled size" << std::endl;
      exit(0);
    }
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, td);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::vpbroadcastb(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t b = 0;
  uint256_t sourcea = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  b = sourcea & 0xff;
  for (int i = 0; i < destop->size; i++) {
    result |= (b << (i * 8));
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    if (destop->size == 16) {
      result &= ((uint256_t(1) << 128) - 1);
      setreg(mapToRegStruct[cs_reg_name(handle, destop->reg)]->parent, result);
    } else {
      setreg(cs_reg_name(handle, destop->reg), result);
    }
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::sete(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t b;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  b = zf;
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), b);
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, b);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 0;
}
int Cpu::movd(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint64_t sourcea = 0;
  uint256_t td = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    if (src_op->size < 16) {
      td = uint256_t(getreg(cs_reg_name(handle, src_op->reg)));
    } else {
      td = getymm(cs_reg_name(handle, src_op->reg));
    }
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &td);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    if (destop->size >= 16) {
      setreg(cs_reg_name(handle, destop->reg), td);
    } else if (destop->size == 4) {
      sourcea = (uint64_t)td;
      setreg(cs_reg_name(handle, destop->reg), sourcea);
    } else {
      std::cout << "[ERROR] Unhandled size" << std::endl;
      exit(0);
    }
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, td);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::vmovdqa(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    if (destop->size == 16) {
      sourcea &= ((uint256_t(1) << 128) - 1);
      setreg(mapToRegStruct[cs_reg_name(handle, destop->reg)]->parent, sourcea);
    } else {
      setreg(cs_reg_name(handle, destop->reg), sourcea);
    }
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::vpminub(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *source_opa, *source_opb, *destop;
  uint256_t a, b = 0;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 3);
  destop = &(x86->operands[0]);
  source_opb = &(x86->operands[1]);
  source_opa = &(x86->operands[2]);
  switch ((int)source_opa->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, source_opa->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(source_opa, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)source_opa->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)source_opb->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, source_opb->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)source_opb->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  for (int i = 0; i < destop->size; i++) {
    a = sourcea & 0xff;
    b = sourceb & 0xff;
    sourcea >>= 8;
    sourceb >>= 8;
    if (a > b) {
      result |= (b << (i * 8));
    } else {
      result |= (a << (i * 8));
    }
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    if (destop->size == 16) {
      result &= ((uint256_t(1) << 128) - 1);
      setreg(mapToRegStruct[cs_reg_name(handle, destop->reg)]->parent, result);
    } else {
      setreg(cs_reg_name(handle, destop->reg), result);
    }
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::cmovb(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t value = 0;
  assert(insn != NULL);
  _rip += insn->size;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &value);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    if (cf == 0) {
      value = getreg(cs_reg_name(handle, writeop->reg));
    }
    if (x86->prefix[2] == 0x66) {
      setreg(cs_reg_name(handle, writeop->reg), value);
    } else {
      if (writeop->size == 4) {
        setreg(mapToReg64(cs_reg_name(handle, writeop->reg)), value);
      } else {
        setreg(cs_reg_name(handle, writeop->reg), value);
      }
    }
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 1;
}
int Cpu::bsr(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t dest = 0;
  uint64_t source = 0;
  assert(insn != NULL);
  _rip += insn->size;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    source = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &source);
    break;
  default:
    std::cout << "[ERROR] Unhandled read operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (source == 0) {
    zf = 1;
    dest = 0;
  } else {
    zf = 0;
    for (dest = (readop->size * 8) - 1; dest >= 0; dest--) {
      if ((source >> dest) & 1) {
        break;
      }
    }
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, writeop->reg), dest);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 1;
}
int Cpu::rol(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t bitcount = 0;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    bitcount = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    bitcount = readop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch (writeop->size) {
  case 1:
  case 2:
  case 4:
    bitcount = (bitcount & 0x1f) % (writeop->size * 8);
    break;
  case 8:
    bitcount = (bitcount & 0x3f) % (writeop->size * 8);
    break;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, writeop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(writeop, &value);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  uint64_t tempCF;
  for (int i = 0; i < bitcount; i++) {
    tempCF = MSB(value, writeop->size * 8);
    value <<= 1;
    value |= tempCF;
    if (writeop->size != 8) {
      value &= ((1ULL << (writeop->size * 8)) - 1);
    }
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, writeop->reg), value);
    break;
  case X86_OP_MEM:
    write_mem_operand(writeop, value);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (bitcount == 0) {
    return 1;
  }
  cf = tempCF;
  of = MSB(value, writeop->size * 8) ^ cf;
  return 1;
}
int Cpu::setg(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t b;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  if (zf == 0 && sf == of) {
    b = 1;
  } else {
    b = 0;
  }
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), b);
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, b);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 0;
}
int Cpu::cqo(cs_insn *insn) {
  cs_detail *detail;
  uint64_t value = 0;
  uint64_t bit = 0;
  uint64_t rdxval = 0;
  assert(insn != NULL);
  detail = insn->detail;
  _rip += insn->size;
  value = getreg("rax");
  bit = MSB(value, 64);
  if (bit) {
    rdxval = -1;
  }
  setreg("rdx", rdxval);
  return 1;
}
int Cpu::idiv(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *denom_op;
  uint64_t result = 0;
  uint64_t denominator = 0;
  uint256_t numerator = 0;
  uint256_t tempdenom = 0;
  uint256_t tempresult = 0;
  uint256_t temprem = 0;
  uint64_t is_negative = 0;
  uint64_t num_is_negative = 0;
  uint64_t lower = 0;
  uint64_t upper = 0;
  uint64_t rem = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  denom_op = &(x86->operands[0]);
  switch ((int)denom_op->type) {
  case X86_OP_REG:
    denominator = getreg(cs_reg_name(handle, denom_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(denom_op, &denominator);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)denom_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch (denom_op->size) {
  case 1:
    lower = getreg("ax");
    upper = 0;
    break;
  case 2:
    lower = getreg("ax");
    upper = getreg("dx");
    break;
  case 4:
    lower = getreg("eax");
    upper = getreg("edx");
    break;
  case 8:
    lower = getreg("rax");
    upper = getreg("rdx");
    break;
  default:
    std::cout << "[ERROR] Unhandled size: " << (int)denom_op->size << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
  };
  if (denominator == 0) {
    std::cout << "[EXCEPTION] divide by zero" << std::endl;
    exit(0);
  }
  numerator = uint256_t(lower) | (uint256_t(upper) << (denom_op->size * 8));
  if (denom_op->size != 8) {
    numerator = SE(numerator, denom_op->size * 16);
  } else {
    numerator = SE(numerator, 128);
  }
  tempdenom = uint256_t(denominator);
  tempdenom = SE(tempdenom, denom_op->size * 8);
  if (MSB(numerator, 256) == 1) {
    is_negative = !is_negative;
    numerator = ~numerator + 1;
    num_is_negative = 1;
  }
  if (MSB(tempdenom, 256) == 1) {
    is_negative = !is_negative;
    tempdenom = ~tempdenom + 1;
  }
  tempresult = numerator / tempdenom;
  temprem = numerator % tempdenom;
  if (is_negative) {
    tempresult = ~tempresult + 1;
  }
  if (num_is_negative) {
    temprem = ~temprem + 1;
  }
  sign_extend(denominator, &denominator, denom_op->size * 8);
  result = tempresult & 0xffffffffffffffff;
  rem = temprem & 0xffffffffffffffff;
  switch (denom_op->size) {
  case 1:
    if (is_negative && (result > 0xffffffffffffff80)) {
      throw FloatingPointException((uint64_t)is_negative);
    } else if (result > 0x7f) {
      throw FloatingPointException((uint64_t)is_negative);
    }
    setreg("al", (uint64_t)(result & 0xff));
    setreg("ah", (uint64_t)(rem & 0xff));
    break;
  case 2:
    if (is_negative && (result > 0xffffffffffff8000)) {
      throw FloatingPointException((uint64_t)is_negative);
    } else if (result > 0x7fff) {
      throw FloatingPointException((uint64_t)is_negative);
    }
    setreg("ax", (uint64_t)(result & 0xffff));
    setreg("dx", (uint64_t)(rem & 0xffff));
    break;
  case 4:
    if (is_negative && (result > 0xffffffff80000000)) {
      throw FloatingPointException((uint64_t)is_negative);
    } else if (result > 0x7fffffff) {
      throw FloatingPointException((uint64_t)is_negative);
    }
    setreg("eax", (uint64_t)(result & 0xffffffff));
    setreg("edx", (uint64_t)(rem & 0xffffffff));
    break;
  case 8:
    if (is_negative && (result > 0x8000000000000000)) {
      throw FloatingPointException((uint64_t)is_negative);
    } else if (result > 0x7fffffffffffffff) {
      throw FloatingPointException((uint64_t)is_negative);
    }
    setreg("rax", (uint64_t)(result & 0xffffffffffffffff));
    setreg("rdx", (uint64_t)(rem & 0xffffffffffffffff));
    break;
  default:
    std::cout << "[ERROR] Unhandled size: " << (int)denom_op->size << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
  };
  return 1;
}
int Cpu::movdqu(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op;
  uint256_t sourcea = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  destop = &(x86->operands[0]);
  src_op = &(x86->operands[1]);
  switch ((int)src_op->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op, &sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), sourcea);
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, sourcea);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand b: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::vpor(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop, *src_op_a, *src_op_b;
  uint256_t sourcea = 0;
  uint256_t sourceb = 0;
  uint256_t result = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 3);
  destop = &(x86->operands[0]);
  src_op_a = &(x86->operands[1]);
  src_op_b = &(x86->operands[2]);
  switch ((int)src_op_a->type) {
  case X86_OP_REG:
    sourcea = getymm(cs_reg_name(handle, src_op_a->reg));
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand a: " << (int)src_op_a->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)src_op_b->type) {
  case X86_OP_REG:
    sourceb = getymm(cs_reg_name(handle, src_op_b->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(src_op_b, &sourceb);
    break;
  default:
    std::cout << "[ERROR] Unhandled source operand b: " << (int)src_op_b->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  result = sourcea | sourceb;
  if (destop->size == 16) {
    result &= ((uint256_t(1) << 128) - 1);
  }
  switch ((int)destop->type) {
  case X86_OP_REG:
    if (destop->size == 16) {
      setreg(mapToRegStruct[cs_reg_name(handle, destop->reg)]->parent, result);
    }
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  default:
    std::cout << "[ERROR] Unhandled dest operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  printExtendedState();
  return 0;
}
int Cpu::jl(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_IMM:
    value = destop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (sf != of) {
    _rip.val = value;
  }
  return 0;
}
int Cpu::cmovs(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t value = 0;
  assert(insn != NULL);
  _rip += insn->size;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &value);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    if (!(sf == 0)) {
      value = getreg(cs_reg_name(handle, writeop->reg));
    }
    if (x86->prefix[2] == 0x66) {
      setreg(cs_reg_name(handle, writeop->reg), value);
    } else {
      if (writeop->size == 4) {
        setreg(mapToReg64(cs_reg_name(handle, writeop->reg)), value);
      } else {
        setreg(cs_reg_name(handle, writeop->reg), value);
      }
    }
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 1;
}
int Cpu::cmovbe(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t value = 0;
  assert(insn != NULL);
  _rip += insn->size;
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(readop, &value);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    if (!((cf == 1) | (zf == 1))) {
      value = getreg(cs_reg_name(handle, writeop->reg));
    }
    if (x86->prefix[2] == 0x66) {
      setreg(cs_reg_name(handle, writeop->reg), value);
    } else {
      if (writeop->size == 4) {
        setreg(mapToReg64(cs_reg_name(handle, writeop->reg)), value);
      } else {
        setreg(cs_reg_name(handle, writeop->reg), value);
      }
    }
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 1;
}
int Cpu::jge(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *destop;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_IMM:
    value = destop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (sf == of) {
    _rip.val = value;
  }
  return 0;
}
int Cpu::leave(cs_insn *insn) {
  cs_detail *detail;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  _rip += insn->size;
  if (detail->regs_read_count != 2) {
    std::cout << "[ERROR] invalid assumption" << std::endl;
    exit(0);
  }
  value = getreg(cs_reg_name(handle, detail->regs_read[0]));
  setreg(cs_reg_name(handle, detail->regs_read[1]), value);
  if (mem->readint((void *)_rsp.val, &value, 8) == -1) {
    std::cout << "[ERROR] segfault attempted read of 0x" << std::hex << _rsp.val
              << std::endl;
    exit(0);
  }
  _rsp.val += 8;
  setreg(cs_reg_name(handle, detail->regs_read[0]), value);
  return 0;
}
int Cpu::ror(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  cs_x86_op *readop, *writeop;
  uint64_t bitcount = 0;
  uint64_t value = 0;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 2);
  writeop = &(x86->operands[0]);
  readop = &(x86->operands[1]);
  switch ((int)readop->type) {
  case X86_OP_REG:
    bitcount = getreg(cs_reg_name(handle, readop->reg));
    break;
  case X86_OP_IMM:
    bitcount = readop->imm;
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)readop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  switch (writeop->size) {
  case 1:
  case 2:
  case 4:
    bitcount = (bitcount & 0x1f) % (writeop->size * 8);
    break;
  case 8:
    bitcount = (bitcount & 0x3f) % (writeop->size * 8);
    break;
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    value = getreg(cs_reg_name(handle, writeop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(writeop, &value);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  uint64_t tempCF;
  for (int i = 0; i < bitcount; i++) {
    tempCF = value & 1;
    value >>= 1;
    value |= tempCF << ((writeop->size * 8) - 1);
    if (writeop->size != 8) {
      value &= ((1ULL << (writeop->size * 8)) - 1);
    }
  }
  switch ((int)writeop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, writeop->reg), value);
    break;
  case X86_OP_MEM:
    write_mem_operand(writeop, value);
    break;
  default:
    std::cout << "[ERROR] Unhandled destination operand: " << (int)writeop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  if (bitcount == 0) {
    return 1;
  }
  cf = tempCF;
  of = ((value >> ((writeop->size * 8) - 2)) & 1) ^ cf;
  return 1;
}
int Cpu::not_ins(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  uint64_t destval = 0;
  uint64_t result = 0;
  cs_x86_op *destop;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    destval = getreg(cs_reg_name(handle, destop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(destop, &destval);
    break;
  default:
    std::cout << "[ERROR] Unhandled read operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sign_extend(destval, &destval, destop->size * 8);
  result = ~destval;
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, result);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  return 1;
}
int Cpu::inc(cs_insn *insn) {
  cs_detail *detail;
  cs_x86 *x86;
  uint64_t destval = 0;
  uint64_t result = 0;
  cs_x86_op *destop;
  assert(insn != NULL);
  detail = insn->detail;
  x86 = &(insn->detail->x86);
  _rip += insn->size;
  opcount_check(insn, 1);
  destop = &(x86->operands[0]);
  switch ((int)destop->type) {
  case X86_OP_REG:
    destval = getreg(cs_reg_name(handle, destop->reg));
    break;
  case X86_OP_MEM:
    read_mem_operand(destop, &destval);
    break;
  default:
    std::cout << "[ERROR] Unhandled read operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  sign_extend(destval, &destval, destop->size * 8);
  result = destval + 1;
  switch ((int)destop->type) {
  case X86_OP_REG:
    setreg(cs_reg_name(handle, destop->reg), result);
    break;
  case X86_OP_MEM:
    write_mem_operand(destop, result);
    break;
  default:
    std::cout << "[ERROR] Unhandled write operand: " << (int)destop->type
              << std::endl;
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn->address, insn->mnemonic,
           insn->op_str, insn->id);
    exit(0);
    break;
  }
  of = (result == ~(1ULL << ((destop->size * 8) - 1)));
  sf = (result >> ((destop->size * 8) - 1)) & 1;
  af = ((result & 0x0f) == 0x0f);
  zf = (result == 0);
  pf = setpf(result);
  return 1;
}
