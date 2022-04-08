#include "cpu.hpp"
rtype x64_regs_list[] = {
    {"al", "rax", LOWBYTE, 8},       {"ah", "rax", HIGHBYTE, 8},
    {"ax", "rax", WORD, 16},         {"eax", "rax", DWORD, 32},
    {"rax", "rax", QWORD, 64},       {"bl", "rbx", LOWBYTE, 8},
    {"bh", "rbx", HIGHBYTE, 8},      {"bx", "rbx", WORD, 16},
    {"ebx", "rbx", DWORD, 32},       {"rbx", "rbx", QWORD, 64},
    {"cl", "rcx", LOWBYTE, 8},       {"ch", "rcx", HIGHBYTE, 8},
    {"cx", "rcx", WORD, 16},         {"ecx", "rcx", DWORD, 32},
    {"rcx", "rcx", QWORD, 64},       {"dl", "rdx", LOWBYTE, 8},
    {"dh", "rdx", HIGHBYTE, 8},      {"dx", "rdx", WORD, 16},
    {"edx", "rdx", DWORD, 32},       {"rdx", "rdx", QWORD, 64},
    {"sil", "rsi", LOWBYTE, 8},      {"si", "rsi", WORD, 16},
    {"esi", "rsi", DWORD, 32},       {"rsi", "rsi", QWORD, 64},
    {"dil", "rdi", LOWBYTE, 8},      {"di", "rdi", WORD, 16},
    {"edi", "rdi", DWORD, 32},       {"rdi", "rdi", QWORD, 64},
    {"spl", "rsp", LOWBYTE, 8},      {"sp", "rsp", WORD, 16},
    {"esp", "rsp", DWORD, 32},       {"rsp", "rsp", QWORD, 64},
    {"bpl", "rbp", LOWBYTE, 8},      {"bp", "rbp", WORD, 16},
    {"ebp", "rbp", DWORD, 32},       {"rbp", "rbp", QWORD, 64},
    {"r8b", "r8", LOWBYTE, 8},       {"r8w", "r8", WORD, 16},
    {"r8d", "r8", DWORD, 32},        {"r8", "r8", QWORD, 64},
    {"r9b", "r9", LOWBYTE, 8},       {"r9w", "r9", WORD, 16},
    {"r9d", "r9", DWORD, 32},        {"r9", "r9", QWORD, 64},
    {"r10b", "r10", LOWBYTE, 8},     {"r10w", "r10", WORD, 16},
    {"r10d", "r10", DWORD, 32},      {"r10", "r10", QWORD, 64},
    {"r11b", "r11", LOWBYTE, 8},     {"r11w", "r11", WORD, 16},
    {"r11d", "r11", DWORD, 32},      {"r11", "r11", QWORD, 64},
    {"r12b", "r12", LOWBYTE, 8},     {"r12w", "r12", WORD, 16},
    {"r12d", "r12", DWORD, 32},      {"r12", "r12", QWORD, 64},
    {"r13b", "r13", LOWBYTE, 8},     {"r13w", "r13", WORD, 16},
    {"r13d", "r13", DWORD, 32},      {"r13", "r13", QWORD, 64},
    {"r14b", "r14", LOWBYTE, 8},     {"r14w", "r14", WORD, 16},
    {"r14d", "r14", DWORD, 32},      {"r14", "r14", QWORD, 64},
    {"r15b", "r15", LOWBYTE, 8},     {"r15w", "r15", WORD, 16},
    {"r15d", "r15", DWORD, 32},      {"r15", "r15", QWORD, 64},
    {"ip", "rip", WORD, 16},         {"eip", "rip", DWORD, 32},
    {"rip", "rip", QWORD, 64},       {"xmm0", "ymm0", DQWORD, 128},
    {"ymm0", "ymm0", QQWORD, 256},   {"xmm1", "ymm1", DQWORD, 128},
    {"ymm1", "ymm1", QQWORD, 256},   {"xmm2", "ymm2", DQWORD, 128},
    {"ymm2", "ymm2", QQWORD, 256},   {"xmm3", "ymm3", DQWORD, 128},
    {"ymm3", "ymm3", QQWORD, 256},   {"xmm4", "ymm4", DQWORD, 128},
    {"ymm4", "ymm4", QQWORD, 256},   {"xmm5", "ymm5", DQWORD, 128},
    {"ymm5", "ymm5", QQWORD, 256},   {"xmm6", "ymm6", DQWORD, 128},
    {"ymm6", "ymm6", QQWORD, 256},   {"xmm7", "ymm7", DQWORD, 128},
    {"ymm7", "ymm7", QQWORD, 256},   {"xmm8", "ymm8", DQWORD, 128},
    {"ymm8", "ymm8", QQWORD, 256},   {"xmm9", "ymm9", DQWORD, 128},
    {"ymm9", "ymm9", QQWORD, 256},   {"xmm10", "ymm10", DQWORD, 128},
    {"ymm10", "ymm10", QQWORD, 256}, {"xmm11", "ymm11", DQWORD, 128},
    {"ymm11", "ymm11", QQWORD, 256}, {"xmm12", "ymm12", DQWORD, 128},
    {"ymm12", "ymm12", QQWORD, 256}, {"xmm13", "ymm13", DQWORD, 128},
    {"ymm13", "ymm13", QQWORD, 256}, {"xmm14", "ymm14", DQWORD, 128},
    {"ymm14", "ymm14", QQWORD, 256}, {"xmm15", "ymm15", DQWORD, 128},
    {"ymm15", "ymm15", QQWORD, 256}, {"xmm16", "ymm16", DQWORD, 128},
    {"ymm16", "ymm16", QQWORD, 256}, {"xmm17", "ymm17", DQWORD, 128},
    {"ymm17", "ymm17", QQWORD, 256}, {"xmm18", "ymm18", DQWORD, 128},
    {"ymm18", "ymm18", QQWORD, 256}, {"xmm19", "ymm19", DQWORD, 128},
    {"ymm19", "ymm19", QQWORD, 256}, {"xmm20", "ymm20", DQWORD, 128},
    {"ymm20", "ymm20", QQWORD, 256}, {"xmm21", "ymm21", DQWORD, 128},
    {"ymm21", "ymm21", QQWORD, 256}, {"xmm22", "ymm22", DQWORD, 128},
    {"ymm22", "ymm22", QQWORD, 256}, {"xmm23", "ymm23", DQWORD, 128},
    {"ymm23", "ymm23", QQWORD, 256}, {"xmm24", "ymm24", DQWORD, 128},
    {"ymm24", "ymm24", QQWORD, 256}, {"xmm25", "ymm25", DQWORD, 128},
    {"ymm25", "ymm25", QQWORD, 256}, {"xmm26", "ymm26", DQWORD, 128},
    {"ymm26", "ymm26", QQWORD, 256}, {"xmm27", "ymm27", DQWORD, 128},
    {"ymm27", "ymm27", QQWORD, 256}, {"xmm28", "ymm28", DQWORD, 128},
    {"ymm28", "ymm28", QQWORD, 256}, {"xmm29", "ymm29", DQWORD, 128},
    {"ymm29", "ymm29", QQWORD, 256}, {"xmm30", "ymm30", DQWORD, 128},
    {"ymm30", "ymm30", QQWORD, 256}, {"xmm31", "ymm31", DQWORD, 128},
    {"ymm31", "ymm31", QQWORD, 256}, {"rflags", "rflags", QWORD, 64},
    {"cs", "cs", WORD, 16},          {"ds", "ds", WORD, 16},
    {"ss", "ss", WORD, 16},          {"es", "es", WORD, 16},
    {"fs", "fs", WORD, 16},          {"gs", "gs", WORD, 16},
};
Cpu::~Cpu() { cs_close(&handle); }
Cpu::Cpu() {
  if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
    std::cout << "[ERROR] Failed to open" << std::endl;
    exit(0);
  }
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
  rdtsc_value = 0;
  setupRegMap();
  setupRegClassMap();
  fs_segment = NULL;
  gs_segment = NULL;
  of = 0;
  cf = 0;
  zf = 0;
  pf = 0;
  af = 0;
  sf = 0;
  tf = 0;
  df = 0;
  iff = 1;
  return;
}
void Cpu::setupRegClassMap() {
  mapToRegClass["rax"] = &_rax;
  mapToRegClass["rcx"] = &_rcx;
  mapToRegClass["rdx"] = &_rdx;
  mapToRegClass["rbx"] = &_rbx;
  mapToRegClass["rsp"] = &_rsp;
  mapToRegClass["rbp"] = &_rbp;
  mapToRegClass["rdi"] = &_rdi;
  mapToRegClass["rsi"] = &_rsi;
  mapToRegClass["rip"] = &_rip;
  mapToRegClass["r8"] = &_r8;
  mapToRegClass["r9"] = &_r9;
  mapToRegClass["r10"] = &_r10;
  mapToRegClass["r11"] = &_r11;
  mapToRegClass["r12"] = &_r12;
  mapToRegClass["r13"] = &_r13;
  mapToRegClass["r14"] = &_r14;
  mapToRegClass["r15"] = &_r15;
  mapToRegClass["ymm0"] = &_ymm0;
  mapToRegClass["ymm1"] = &_ymm1;
  mapToRegClass["ymm2"] = &_ymm2;
  mapToRegClass["ymm3"] = &_ymm3;
  mapToRegClass["ymm4"] = &_ymm4;
  mapToRegClass["ymm5"] = &_ymm5;
  mapToRegClass["ymm6"] = &_ymm6;
  mapToRegClass["ymm7"] = &_ymm7;
  mapToRegClass["ymm8"] = &_ymm8;
  mapToRegClass["ymm9"] = &_ymm9;
  mapToRegClass["ymm10"] = &_ymm10;
  mapToRegClass["ymm11"] = &_ymm11;
  mapToRegClass["ymm12"] = &_ymm12;
  mapToRegClass["ymm13"] = &_ymm13;
  mapToRegClass["ymm14"] = &_ymm14;
  mapToRegClass["ymm15"] = &_ymm15;
  mapToRegClass["ymm16"] = &_ymm16;
  mapToRegClass["ymm17"] = &_ymm17;
  mapToRegClass["ymm18"] = &_ymm18;
  mapToRegClass["ymm19"] = &_ymm19;
  mapToRegClass["ymm20"] = &_ymm20;
  mapToRegClass["ymm21"] = &_ymm21;
  mapToRegClass["ymm22"] = &_ymm22;
  mapToRegClass["ymm23"] = &_ymm23;
  mapToRegClass["ymm24"] = &_ymm24;
  mapToRegClass["ymm25"] = &_ymm25;
  mapToRegClass["ymm26"] = &_ymm26;
  mapToRegClass["ymm27"] = &_ymm27;
  mapToRegClass["ymm28"] = &_ymm28;
  mapToRegClass["ymm29"] = &_ymm29;
  mapToRegClass["ymm30"] = &_ymm30;
  mapToRegClass["ymm31"] = &_ymm31;
  mapToRegClass["rflags"] = &_rflags;
  mapToRegClass["cs"] = &_cs;
  mapToRegClass["ds"] = &_ds;
  mapToRegClass["ss"] = &_ss;
  mapToRegClass["es"] = &_es;
  mapToRegClass["fs"] = &_fs;
  mapToRegClass["gs"] = &_gs;
  for (int i = 0; i < sizeof(x64_regs_list) / sizeof(rtype); i++) {
    mapToRegStruct[x64_regs_list[i].n] = &x64_regs_list[i];
  }
  for (int i = 0; i < sizeof(x64_regs_list) / sizeof(rtype); i++) {
    auto it = mapToRegClass.find(x64_regs_list[i].n);
    if (it != mapToRegClass.end()) {
      it->second->name = x64_regs_list[i].n;
      it->second->bits = x64_regs_list[i].bits;
    }
  }
  return;
}
Cpu::Cpu(Memory *mem) {
  this->mem = mem;
  if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
    std::cout << "[ERROR] Failed to open" << std::endl;
    exit(0);
  }
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
  rdtsc_value = 0;
  setupRegMap();
  setupRegClassMap();
  of = 0;
  cf = 0;
  zf = 0;
  pf = 0;
  af = 0;
  sf = 0;
  tf = 0;
  iff = 1;
  return;
}
int Cpu::read_segment_mem(std::string reg, uint64_t offset, char *value,
                          uint64_t length) {
  char *addr = 0;
  uint8_t b;
  if (value == NULL) {
    return -1;
  }
  if (cppstrncasecmp(reg, "fs") == true) {
    addr = (char *)(reinterpret_cast<uint64_t>(fs_segment) + offset);
    if (!mem->isReadable(reinterpret_cast<uint64_t>(addr), length)) {
      throw MemoryException();
    }
    for (int i = 0; i < length; i++) {
      b = addr[i] & 0xff;
      value[i] = b;
    }
    return 0;
  } else if (cppstrncasecmp(reg, "gs") == true) {
    addr = (char *)(gs_segment) + offset;
    if (!mem->isReadable(reinterpret_cast<uint64_t>(addr), length)) {
      throw MemoryException();
    }
    for (int i = 0; i < length; i++) {
      value[i] = addr[i];
    }
    return 0;
  } else {
    std::cout << "[ERROR] Invalid segment register: " << reg << std::endl;
    exit(0);
  }
  return 0;
}
int Cpu::write_segment_mem(std::string reg, uint64_t offset, char *data,
                           uint64_t length) {
  char *addr = 0;
  if (cppstrncasecmp(reg, "fs") == true) {
    addr = (char *)(reinterpret_cast<uint64_t>(fs_segment) + offset);
    if (!mem->isWriteable(reinterpret_cast<uint64_t>(addr), length)) {
      throw MemoryException();
    }
    for (int i = 0; i < length; i++) {
      addr[i] = data[i];
    }
    return 0;
  } else if (cppstrncasecmp(reg, "gs") == true) {
    addr = (char *)(reinterpret_cast<uint64_t>(gs_segment) + offset);
    if (!mem->isWriteable(reinterpret_cast<uint64_t>(addr), length)) {
      throw MemoryException();
    }
    for (int i = 0; i < length; i++) {
      addr[i] = data[i];
    }
    return 0;
  } else {
    std::cout << "[ERROR] Invalid segment register: " << reg << std::endl;
    exit(0);
  }
  return 0;
}
void Cpu::init_fs_segment(void) {
  uint64_t cookie = 0xdeadbeefcafebabe;
  if (mem == NULL) {
    std::cout << "[ERROR] No memory model" << std::endl;
    exit(0);
  }
  void *fss = mem->mmap_l(NULL, 0x1000, PROT_READ | PROT_WRITE,
                          MAP_ANON | MAP_PRIVATE, 0, 0);
  if (fss == NULL) {
    std::cout << "[ERROR] mmap()" << std::endl;
    exit(0);
  }
  void *gss = mem->mmap_l(NULL, 0x1000, PROT_READ | PROT_WRITE,
                          MAP_ANON | MAP_PRIVATE, 0, 0);
  if (gss == NULL) {
    std::cout << "[ERROR] mmap()" << std::endl;
    exit(0);
  }
  fs_segment = ((char *)fss) + 0x800;
  gs_segment = ((char *)gss) + 0x800;
  write_segment_mem("fs", 0x28, (char *)&cookie, sizeof(cookie));
  return;
}
void Cpu::setupRegMap() {
  regToReg64["pc"] = "rip";
  regToReg64["ip"] = "rip";
  regToReg64["eip"] = "rip";
  regToReg64["rip"] = "rip";
  regToReg64["al"] = "rax";
  regToReg64["ah"] = "rax";
  regToReg64["ax"] = "rax";
  regToReg64["eax"] = "rax";
  regToReg64["rax"] = "rax";
  regToReg64["bl"] = "rbx";
  regToReg64["bh"] = "rbx";
  regToReg64["bx"] = "rbx";
  regToReg64["ebx"] = "rbx";
  regToReg64["rbx"] = "rbx";
  regToReg64["cl"] = "rcx";
  regToReg64["ch"] = "rcx";
  regToReg64["cx"] = "rcx";
  regToReg64["ecx"] = "rcx";
  regToReg64["rcx"] = "rcx";
  regToReg64["dl"] = "rdx";
  regToReg64["dh"] = "rdx";
  regToReg64["dx"] = "rdx";
  regToReg64["edx"] = "rdx";
  regToReg64["rdx"] = "rdx";
  regToReg64["sil"] = "rsi";
  regToReg64["si"] = "rsi";
  regToReg64["esi"] = "rsi";
  regToReg64["rsi"] = "rsi";
  regToReg64["dil"] = "rdi";
  regToReg64["di"] = "rdi";
  regToReg64["edi"] = "rdi";
  regToReg64["rdi"] = "rdi";
  regToReg64["spl"] = "rsp";
  regToReg64["sp"] = "rsp";
  regToReg64["esp"] = "rsp";
  regToReg64["rsp"] = "rsp";
  regToReg64["bpl"] = "rbp";
  regToReg64["bp"] = "rbp";
  regToReg64["ebp"] = "rbp";
  regToReg64["rbp"] = "rbp";
  regToReg64["r8b"] = "r8";
  regToReg64["r8w"] = "r8";
  regToReg64["r8d"] = "r8";
  regToReg64["r8"] = "r8";
  regToReg64["r9b"] = "r9";
  regToReg64["r9w"] = "r9";
  regToReg64["r9d"] = "r9";
  regToReg64["r9"] = "r9";
  regToReg64["r10b"] = "r10";
  regToReg64["r10w"] = "r10";
  regToReg64["r10d"] = "r10";
  regToReg64["r10"] = "r10";
  regToReg64["r11b"] = "r11";
  regToReg64["r11w"] = "r11";
  regToReg64["r11d"] = "r11";
  regToReg64["r11"] = "r11";
  regToReg64["r12b"] = "r12";
  regToReg64["r12w"] = "r12";
  regToReg64["r12d"] = "r12";
  regToReg64["r12"] = "r12";
  regToReg64["r13b"] = "r13";
  regToReg64["r13w"] = "r13";
  regToReg64["r13d"] = "r13";
  regToReg64["r13"] = "r13";
  regToReg64["r14b"] = "r14";
  regToReg64["r14w"] = "r14";
  regToReg64["r14d"] = "r14";
  regToReg64["r14"] = "r14";
  regToReg64["r15b"] = "r15";
  regToReg64["r15w"] = "r15";
  regToReg64["r15d"] = "r15";
  regToReg64["r15"] = "r15";
  return;
}
std::string Cpu::mapToReg64(std::string reg) {
  auto it = regToReg64.find(reg);
  if (it != regToReg64.end()) {
    return it->second;
  }
  std::cout << "[ERROR] Unknown register: " << reg << std::endl;
  exit(0);
  return "";
}
bool Cpu::isReg(std::string reg) {
  auto it = regToReg64.find(reg);
  if (it != regToReg64.end()) {
    return true;
  }
  return false;
}
void Cpu::printState() {
  std::cout << std::hex << "rax: 0x" << _rax.val << "\t rcx: 0x" << _rcx.val
            << "\t rdx: 0x" << _rdx.val << "\t rbx: 0x" << _rbx.val
            << std::endl;
  std::cout << std::hex << "rsp: 0x" << _rsp.val << "\t rbp: 0x" << _rbp.val
            << "\t rsi: 0x" << _rsi.val << "\t rdi: 0x" << _rdi.val
            << std::endl;
  std::cout << std::hex << "rip: 0x" << _rip.val << "\t r8: 0x" << _r8.val
            << "\t r9: 0x" << _r9.val << "\t r10: 0x" << _r10.val << std::endl;
  std::cout << std::hex << "r11: 0x" << _r11.val << "\t r12: 0x" << _r12.val
            << "\t r13: 0x" << _r13.val << "\t r14: 0x" << _r14.val
            << std::endl;
  std::cout << std::hex << "r15: 0x" << _r15.val << "\t";
  if (cf) {
    std::cout << " CF";
  }
  if (pf) {
    std::cout << " PF";
  }
  if (af) {
    std::cout << " AF";
  }
  if (zf) {
    std::cout << " ZF";
  }
  if (sf) {
    std::cout << " SF";
  }
  if (tf) {
    std::cout << " TF";
  }
  if (iff) {
    std::cout << " IF";
  }
  if (df) {
    std::cout << " DF";
  }
  if (of) {
    std::cout << " OF";
  }
  std::cout << std::endl << std::endl;
  return;
}
void Cpu::printStateGDB() {
  std::cout << std::hex;
  std::cout << "rax\t0x" << _rax.val << std::endl;
  std::cout << "rbx\t0x" << _rbx.val << std::endl;
  std::cout << "rcx\t0x" << _rcx.val << std::endl;
  std::cout << "rdx\t0x" << _rdx.val << std::endl;
  std::cout << "rsi\t0x" << _rsi.val << std::endl;
  std::cout << "rdi\t0x" << _rdi.val << std::endl;
  std::cout << "rbp\t0x" << _rbp.val << std::endl;
  std::cout << "rsp\t0x" << _rsp.val << std::endl;
  std::cout << "r8\t0x" << _r8.val << std::endl;
  std::cout << "r9\t0x" << _r9.val << std::endl;
  std::cout << "r10\t0x" << _r10.val << std::endl;
  std::cout << "r11\t0x" << _r11.val << std::endl;
  std::cout << "r12\t0x" << _r12.val << std::endl;
  std::cout << "r13\t0x" << _r13.val << std::endl;
  std::cout << "r14\t0x" << _r14.val << std::endl;
  std::cout << "r15\t0x" << _r15.val << std::endl;
  std::cout << "rip\t0x" << _rip.val << std::endl;
  std::cout << "eflags\t";
  if (cf) {
    std::cout << " CF";
  }
  if (pf) {
    std::cout << " PF";
  }
  if (af) {
    std::cout << " AF";
  }
  if (zf) {
    std::cout << " ZF";
  }
  if (sf) {
    std::cout << " SF";
  }
  if (tf) {
    std::cout << " TF";
  }
  if (iff) {
    std::cout << " IF";
  }
  if (df) {
    std::cout << " DF";
  }
  if (of) {
    std::cout << " OF";
  }
  std::cout << std::endl;
  std::cout << "cs\t0x33" << std::endl;
  std::cout << "ss\t0x2b" << std::endl;
  std::cout << "ds\t0x0" << std::endl;
  std::cout << "es\t0x0" << std::endl;
  std::cout << "fs\t0x0" << std::endl;
  std::cout << "gs\t0x0" << std::endl;
  return;
}
void Cpu::printExtendedState() {
  return;
  std::cout << std::hex << "ymm0: 0x" << _ymm0.ymm << "\t ymm1: 0x" << _ymm1.ymm
            << "\t ymm2: 0x" << _ymm2.ymm << "\t ymm3: " << _ymm3.ymm
            << std::endl;
  std::cout << std::hex << "ymm4: 0x" << _ymm4.ymm << "\t ymm5: 0x" << _ymm5.ymm
            << "\t ymm6: 0x" << _ymm6.ymm << "\t ymm7: " << _ymm7.ymm
            << std::endl;
  std::cout << std::hex << "ymm8: 0x" << _ymm8.ymm << "\t ymm9: 0x" << _ymm9.ymm
            << "\t ymm10: 0x" << _ymm10.ymm << "\t ymm11: " << _ymm11.ymm
            << std::endl;
  std::cout << std::hex << "ymm12: 0x" << _ymm12.ymm << "\t ymm13: 0x"
            << _ymm13.ymm << "\t ymm14: 0x" << _ymm14.ymm
            << "\t ymm15: " << _ymm15.ymm << std::endl;
  std::cout << std::hex << "ymm16: 0x" << _ymm16.ymm << "\t";
  std::cout << std::endl << std::endl;
  return;
}
uint64_t Cpu::step(uint64_t print_flag) {
  size_t count;
  char bytes[32];
  memset(bytes, 0, 32);
  if (mem->read_l((void *)_rip.val, bytes, 16) == -1) {
    std::cout << "[EXCEPTION] Sefault trying to read from: 0x" << _rip.val
              << std::endl;
    exit(0);
  }
  count = cs_disasm(handle, (const uint8_t *)bytes, 16, _rip.val, 1, &insn);
  if (count <= 0) {
    std::cout << "[ERROR] invalid instruction at 0x" << _rip.val << std::endl;
    exit(0);
  }
  if (print_flag) {
    printf("0x%lx:\t%s\t\t%s\t%d\n", insn[0].address, insn[0].mnemonic,
           insn[0].op_str, insn[0].id);
  }
  if (!can_exec(_rip.val)) {
    std::cout << "[ERROR] segfault trying to exec: " << _rip.val << std::endl;
    exit(0);
  }
  switch (insn[0].id) {
  case 8:
    add(&insn[0]);
    break;
  case 25:
    and_ins(&insn[0]);
    break;
  case 48:
    bsf(&insn[0]);
    break;
  case 49:
    bsr(&insn[0]);
    break;
  case 51:
    bt(&insn[0]);
    break;
  case 56:
    call(&insn[0]);
    break;
  case 59:
    cdqe(&insn[0]);
    break;
  case 62:
    clc(&insn[0]);
    break;
  case 71:
    cmova(&insn[0]);
    break;
  case 73:
    cmovb(&insn[0]);
    break;
  case 74:
    cmovbe(&insn[0]);
    break;
  case 77:
    cmove(&insn[0]);
    break;
  case 85:
    cmovne(&insn[0]);
    break;
  case 94:
    cmovs(&insn[0]);
    break;
  case 95:
    cmp(&insn[0]);
    break;
  case 100:
    cmpxchg(&insn[0]);
    break;
  case 109:
    cpuid(&insn[0]);
    break;
  case 110:
    cqo(&insn[0]);
    break;
  case 133:
    dec(&insn[0]);
    break;
  case 134:
    div_ins(&insn[0]);
    break;
  case 147:
    ret(&insn[0]);
    break;
  case 198:
    movaps(&insn[0]);
    break;
  case 211:
    idiv(&insn[0]);
    break;
  case 213:
    imul(&insn[0]);
    break;
  case 215:
    inc(&insn[0]);
    break;
  case 253:
    jae(&insn[0]);
    break;
  case 254:
    ja(&insn[0]);
    break;
  case 255:
    jbe(&insn[0]);
    break;
  case 256:
    jb(&insn[0]);
    break;
  case 259:
    je(&insn[0]);
    break;
  case 260:
    jge(&insn[0]);
    break;
  case 261:
    jg(&insn[0]);
    break;
  case 262:
    jle(&insn[0]);
    break;
  case 263:
    jl(&insn[0]);
    break;
  case 264:
    jmp(&insn[0]);
    break;
  case 265:
    jne(&insn[0]);
    break;
  case 272:
    js(&insn[0]);
    break;
  case 322:
    lea(&insn[0]);
    break;
  case 323:
    leave(&insn[0]);
    break;
  case 332:
    or_ins(&insn[0]);
    break;
  case 333:
    sub(&insn[0]);
    break;
  case 334:
    xor_ins(&insn[0]);
    break;
  case 367:
    movd(&insn[0]);
    break;
  case 371:
    movq(&insn[0]);
    break;
  case 379:
    paddd(&insn[0]);
    break;
  case 380:
    paddq(&insn[0]);
    break;
  case 391:
    pcmpeqb(&insn[0]);
    break;
  case 392:
    pcmpeqd(&insn[0]);
    break;
  case 395:
    pcmpgtd(&insn[0]);
    break;
  case 411:
    pmovmskb(&insn[0]);
    break;
  case 424:
    pslld(&insn[0]);
    break;
  case 425:
    psllq(&insn[0]);
    break;
  case 432:
    psubb(&insn[0]);
    break;
  case 441:
    punpckhdq(&insn[0]);
    break;
  case 444:
    punpckldq(&insn[0]);
    break;
  case 446:
    pxor(&insn[0]);
    break;
  case 449:
    mov(&insn[0]);
    break;
  case 450:
    mov(&insn[0]);
    break;
  case 453:
    movdqa(&insn[0]);
    break;
  case 454:
    movdqu(&insn[0]);
    break;
  case 475:
    movss(&insn[0]);
    break;
  case 477:
    movsx(&insn[0]);
    break;
  case 478:
    movsxd(&insn[0]);
    break;
  case 480:
    movups(&insn[0]);
    break;
  case 481:
    movzx(&insn[0]);
    break;
  case 483:
    mul(&insn[0]);
    break;
  case 493:
    neg(&insn[0]);
    break;
  case 494:
    nop(&insn[0]);
    break;
  case 495:
    not_ins(&insn[0]);
    break;
  case 566:
    pop(&insn[0]);
    break;
  case 582:
    pslldq(&insn[0]);
    break;
  case 586:
    punpckhqdq(&insn[0]);
    break;
  case 587:
    punpcklqdq(&insn[0]);
    break;
  case 588:
    push(&insn[0]);
    break;
  case 604:
    rdtsc(&insn[0]);
    break;
  case 606:
    rol(&insn[0]);
    break;
  case 607:
    ror(&insn[0]);
    break;
  case 619:
    sar(&insn[0]);
    break;
  case 621:
    sbb(&insn[0]);
    break;
  case 627:
    seta(&insn[0]);
    break;
  case 629:
    setb(&insn[0]);
    break;
  case 630:
    sete(&insn[0]);
    break;
  case 632:
    setg(&insn[0]);
    break;
  case 635:
    setne(&insn[0]);
    break;
  case 651:
    shl(&insn[0]);
    break;
  case 654:
    shr(&insn[0]);
    break;
  case 670:
    stc(&insn[0]);
    break;
  case 677:
    stosq(&insn[0]);
    break;
  case 695:
    syscall(&insn[0]);
    break;
  case 700:
    test(&insn[0]);
    break;
  case 703:
    tzcnt(&insn[0]);
    break;
  case 924:
    vmovd(&insn[0]);
    break;
  case 927:
    vmovdqa(&insn[0]);
    break;
  case 932:
    vmovdqu(&insn[0]);
    break;
  case 997:
    vpbroadcastb(&insn[0]);
    break;
  case 1007:
    vpcmpeqb(&insn[0]);
    break;
  case 1121:
    vpminub(&insn[0]);
    break;
  case 1131:
    vpmovmskb(&insn[0]);
    break;
  case 1167:
    vpor(&insn[0]);
    break;
  case 1233:
    vpxor(&insn[0]);
    break;
  case 1292:
    vzeroupper(&insn[0]);
    break;
  case 1308:
    xgetbv(&insn[0]);
    break;
  default:
    throw InstructionException();
    break;
  };
  cs_free(insn, count);
  rdtsc_value += 5;
  return 0;
}
uint64_t Cpu::setreg(std::string regname, uint64_t value) {
  rtype *rt = NULL;
  reg *rc = NULL;
  auto it = mapToRegStruct.find(regname);
  if (it == mapToRegStruct.end()) {
    std::cout << "[ERROR] Invalid register: " << regname << std::endl;
    exit(0);
  }
  rt = it->second;
  auto rit = mapToRegClass.find(rt->parent);
  if (rit == mapToRegClass.end()) {
    std::cout << "[ERROR] Invalid parent register: " << rt->parent << std::endl;
    exit(0);
  }
  rc = rit->second;
  rc->set(value, rt->type);
  return value;
}
uint256_t Cpu::setreg(std::string regname, uint256_t value) {
  rtype *rt = NULL;
  reg *rc = NULL;
  auto it = mapToRegStruct.find(regname);
  if (it == mapToRegStruct.end()) {
    std::cout << "[ERROR] Invalid register: " << regname << std::endl;
    exit(0);
  }
  rt = it->second;
  auto rit = mapToRegClass.find(rt->parent);
  if (rit == mapToRegClass.end()) {
    std::cout << "[ERROR] Invalid parent register: " << rt->parent << std::endl;
    exit(0);
  }
  rc = rit->second;
  rc->set(value, uint256_t(rt->type));
  return value;
}
uint64_t Cpu::getreg(std::string regname) {
  rtype *rt = NULL;
  reg *rc = NULL;
  auto it = mapToRegStruct.find(regname);
  if (it == mapToRegStruct.end()) {
    std::cout << "[ERROR] Invalid register: " << regname << std::endl;
    exit(0);
  }
  rt = it->second;
  auto rit = mapToRegClass.find(rt->parent);
  if (rit == mapToRegClass.end()) {
    std::cout << "[ERROR] Invalid parent register: " << rt->parent << std::endl;
    exit(0);
  }
  rc = rit->second;
  return rc->get(rt->type);
}
uint256_t Cpu::getymm(std::string regname) {
  rtype *rt = NULL;
  reg *rc = NULL;
  auto it = mapToRegStruct.find(regname);
  if (it == mapToRegStruct.end()) {
    std::cout << "[ERROR] Invalid register: " << regname << std::endl;
    exit(0);
  }
  rt = it->second;
  auto rit = mapToRegClass.find(rt->parent);
  if (rit == mapToRegClass.end()) {
    std::cout << "[ERROR] Invalid parent register: " << rt->parent << std::endl;
    exit(0);
  }
  rc = rit->second;
  return rc->get(uint256_t(rt->type));
}
