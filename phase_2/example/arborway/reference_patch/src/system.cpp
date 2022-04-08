#include "system.hpp"
#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <iterator>
#include <stdio.h>
#include "syscall_int.hpp"
std::vector<std::string> vectorize_array(char **arr) {
  std::vector<std::string> varr;
  if (arr == NULL) {
    return varr;
  }
  for (int i = 0; arr[i] != NULL; i++) {
    varr.push_back(std::string(arr[i]));
  }
  return varr;
}
void System::load_vdso() {
  char *temp_vdso = NULL;
  FILE *fp = NULL;
  uint64_t size;
  fp = fopen(vdso_filename.c_str(), "r");
  if (fp == NULL) {
    std::cout << "[ERROR] failed to load: " << vdso_filename << std::endl;
    exit(0);
  }
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  temp_vdso = (char *)mem.mmapFile(NULL, size, MAP_ANON | MAP_PRIVATE,
                                   PROT_READ | PROT_WRITE | PROT_EXEC, 0, fp);
  if (temp_vdso == NULL) {
    std::cout << "[ERROR] failed to load a file" << std::endl;
    exit(0);
  }
  mem.mprotect_l(temp_vdso, size, PROT_READ | PROT_EXEC);
  vdso = temp_vdso;
  vdso_length = size;
  return;
}
System::System(char *data, uint64_t size) {
  char cmd[128];
  uint64_t ins_count = 0;
  uint64_t break_instruction = 0;
  uint64_t break_address = 0;
  uint64_t step = 1;
  uint64_t print_ins = 1;
  uint64_t print_regs = 1;
  uint64_t print_ext = 0;
  uint64_t read_addr = 0;
  uint64_t read_value = 0;
  char *shellcode = NULL;
  char *stack = NULL;
  uint64_t l;
  std::vector<std::string> tokenized_cmd;
  shellcode = (char *)mem.mmap_l((void *)0x41410000, mem.ceil(size),
                                 PROT_READ | PROT_WRITE | PROT_EXEC,
                                 MAP_ANON | MAP_PRIVATE, 0, 0);
  if (shellcode == NULL) {
    std::cerr << "[ERROR] Failed to allocate memory" << std::endl;
    exit(0);
  }
  mem.write_l(shellcode, data, size);
  stack = (char *)mem.mmap_l((void *)NULL, 0x10000, PROT_READ | PROT_WRITE,
                             MAP_ANON | MAP_PRIVATE, 0, 0);
  if (stack == NULL) {
    std::cerr << "[ERROR] Failed to allocate memory" << std::endl;
    exit(0);
  }
  stack = stack + (0x10000 - 16);
  cpu.setreg("rsp", reinterpret_cast<uint64_t>(stack));
  cpu.setreg("rip", reinterpret_cast<uint64_t>(shellcode));
  this->cpu.setreg("rax", 0);
  this->cpu.setreg("rbx", 0);
  this->cpu.setreg("rcx", 0);
  this->cpu.setreg("rdx", 0);
  this->cpu.setreg("rbp", 0);
  this->cpu.setreg("rsi", 0);
  this->cpu.setreg("rdi", 0);
  this->cpu.setreg("r8", 0);
  this->cpu.setreg("r9", 0);
  this->cpu.setreg("r10", 0);
  this->cpu.setreg("r11", 0);
  this->cpu.setreg("r12", 0);
  this->cpu.setreg("r13", 0);
  this->cpu.setreg("r14", 0);
  this->cpu.setreg("r15", 0);
  os.cpu = &cpu;
  os.mem = &mem;
  os.elf = "/dev/null";
  cpu.mem = &mem;
  cpu.init_fs_segment();
  std::cout << "[INFO] Executing now..." << std::endl;
  while (1) {
    if (step) {
      std::cout << (int)ins_count << "# ";
      memset(cmd, 0, 128);
      fgets(cmd, 127, stdin);
      l = strlen(cmd);
      if (!l) {
        continue;
      }
      if (cmd[l - 1] == '\n') {
        cmd[l - 1] = 0x00;
      }
      tokenized_cmd = cpptok(std::string(cmd), ' ');
      if (tokenized_cmd[0] == "c") {
        break_instruction = -1;
        break_address = 0;
        step = 0;
      } else if (tokenized_cmd[0] == "g") {
        if (tokenized_cmd.size() != 2) {
          std::cout << "g <address>" << std::endl;
          continue;
        }
        break_address = strtoull(tokenized_cmd[1].c_str(), NULL, 16);
        break_instruction = -1;
        step = 0;
      } else if (tokenized_cmd[0] == "t") {
        if (tokenized_cmd.size() != 2) {
          std::cout << "t <skip count>" << std::endl;
          continue;
        }
        break_address = 0;
        break_instruction =
            ins_count + strtoull(tokenized_cmd[1].c_str(), NULL, 16);
        step = 0;
      } else if (tokenized_cmd[0] == "s") {
        break_address = 0;
        break_instruction = ins_count + 1;
        step = 1;
      } else if (tokenized_cmd[0] == "set") {
        if (tokenized_cmd.size() != 2) {
          std::cout << "set [pr | pe | pi]" << std::endl;
          continue;
        }
        if (tokenized_cmd[1] == "pr") {
          print_regs = 1;
        } else if (tokenized_cmd[1] == "pe") {
          print_ext = 1;
        } else if (tokenized_cmd[1] == "pi") {
          print_ins = 1;
        }
        continue;
      } else if (tokenized_cmd[0] == "unset") {
        if (tokenized_cmd.size() != 2) {
          std::cout << "unset [pr | pe | pi]" << std::endl;
          continue;
        }
        if (tokenized_cmd[1] == "pr") {
          print_regs = 0;
        } else if (tokenized_cmd[1] == "pe") {
          print_ext = 0;
        } else if (tokenized_cmd[1] == "pi") {
          print_ins = 0;
        }
        continue;
      } else if (tokenized_cmd[0] == "x") {
        if (tokenized_cmd.size() != 2) {
          std::cout << "x [<address> | <reg>]" << std::endl;
          continue;
        }
        if (cpu.isReg(tokenized_cmd[1])) {
          read_addr = cpu.getreg(tokenized_cmd[1]);
        } else {
          read_addr = strtoull(tokenized_cmd[1].c_str(), NULL, 16);
        }
        if (mem.isReadable(read_addr, 8)) {
          mem.read_l((void *)read_addr, (char *)&read_value, 8);
          std::cout << "0x" << read_addr << ":\t0x" << std::setfill('0')
                    << std::setw(16) << read_value << std::endl;
        } else {
          std::cout << "0x" << read_addr << ":\tCannot access memory"
                    << std::endl;
        }
        continue;
      } else if (tokenized_cmd[0] == "pm") {
        mem.walkTheBlock();
        continue;
      }
    }
    ins_count++;
    try {
      cpu.step(print_ins);
    } catch (SycallException e) {
      if (os.syscall_dispatch()) {
        cpu.printState();
        std::cout << "[INFO] Total instructions: " << ins_count << std::endl;
        std::cout << "[INFO] Execution Completed" << std::endl;
        int waitforit;
        std::cin >> waitforit;
        std::cerr << "[INFO] Execution Completed" << std::endl;
        return;
      }
    } catch (MemoryException e) {
      std::cout << e.what() << ": " << e.error_number << std::endl;
      cpu.printState();
      return;
    } catch (FloatingPointException e) {
      std::cout << e.what() << ": " << e.error_number << std::endl;
    } catch (InstructionException e) {
      std::cout << e.what() << ": " << e.error_number << std::endl;
      return;
    }
    if (print_regs) {
      cpu.printState();
    }
    if (print_ext) {
      cpu.printExtendedState();
    }
    if (ins_count >= break_instruction || break_address == cpu._rip.val) {
      step = 1;
    }
  }
  return;
}
System::System(std::string filename, std::vector<std::string> envp,
               std::string vdso) {
  elf_ph_64 *eph = NULL;
  uint32_t exec_stack = 0;
  char *elf_interpreter = NULL;
  Elf *interpreter = NULL;
  void *temp_reserve = NULL;
  void *elf_program_headers = NULL;
  uint64_t entry = 0;
  uint64_t memsz = 0;
  uint64_t offset = 0;
  uint64_t filesz = 0;
  std::filesystem::path p;
  void *stack_base;
  this->mem.pagebits = 12;
  vdso_filename = vdso;
  p = std::filesystem::absolute(std::filesystem::path(filename));
  if (this->elf.parse_elf(filename) == 0) {
    std::cout << "[FAIL] exitting..." << std::endl;
    exit(0);
  }
  for (auto it = this->elf.header.begin(); it != this->elf.header.end(); ++it) {
    eph = *it;
    if (eph->p_type != PT_INTERP) {
      continue;
    }
    elf_interpreter = (char *)calloc(1, eph->p_filesz);
    if (elf_interpreter) {
      fseek(this->elf.fp, eph->p_offset, SEEK_SET);
      fread(elf_interpreter, 1, eph->p_filesz, this->elf.fp);
    }
    break;
  }
  if (elf_interpreter) {
    interpreter = new Elf();
    if (interpreter->parse_elf(std::string(elf_interpreter)) == 0) {
      std::cout << "[FAIL] Failed to load the interpreter" << std::endl;
      exit(0);
    }
  }
  for (auto it = this->elf.header.begin(); it != this->elf.header.end(); ++it) {
    eph = *it;
    if (eph->p_type == PT_GNU_STACK) {
      if (eph->p_flags & PF_X) {
        exec_stack = 1;
      }
    }
  }
  uint64_t elf_bss = 0;
  uint64_t elf_brk = 0;
  uint64_t start_code = ~0;
  uint64_t end_code = 0;
  uint64_t start_data = 0;
  uint64_t end_data = 0;
  uint64_t e_entry = 0;
  uint32_t load_addr_set = 0;
  uint64_t load_bias = 0;
  uint64_t vaddr = 0;
  uint64_t k = 0;
  uint64_t prot;
  uint64_t bss_prot;
  uint64_t flags;
  uint64_t pageoffset = 0;
  for (auto it = this->elf.header.begin(); it != this->elf.header.end(); ++it) {
    eph = *it;
    if (eph->p_type != PT_LOAD) {
      continue;
    }
    pageoffset = eph->p_vaddr & ((1 << this->mem.pagebits) - 1);
    flags = MAP_PRIVATE | MAP_ANON;
    vaddr = eph->p_vaddr - pageoffset;
    memsz = this->mem.ceil(eph->p_memsz + pageoffset + 1);
    offset = eph->p_offset - pageoffset;
    filesz = eph->p_filesz + pageoffset;
    if (this->elf.eh.e_type == ET_EXEC || load_addr_set) {
      flags |= MAP_FIXED;
    } else if (this->elf.eh.e_type == ET_DYN) {
      if (interpreter) {
        load_bias = ELF_ET_DYN_BASE;
      } else {
        load_bias = 0;
      }
      load_bias = this->mem.floor(load_bias - vaddr);
    }
    prot = make_prot(eph->p_flags);
    void *r = this->mem.mmapFile(((char *)load_bias) + vaddr, memsz, flags,
                                 prot, offset, this->elf.fp);
    if (!load_addr_set) {
      load_addr_set = 1;
      if (this->elf.eh.e_type == ET_DYN) {
        load_bias = vaddr - offset;
        load_bias += reinterpret_cast<uint64_t>(r);
      }
    }
    k = vaddr;
    if ((eph->p_flags & PF_X) && k < start_code) {
      start_code = k;
    }
    if (start_data < k) {
      start_data = k;
    }
    k = vaddr + filesz;
    if (k > elf_bss) {
      elf_bss = k;
    }
    if ((eph->p_flags & PF_X) && end_code < k) {
      end_code = k;
    }
    if (end_data < k) {
      end_data = k;
    }
    k = vaddr + memsz;
    if (k > elf_brk) {
      bss_prot = prot;
      elf_brk = k;
    }
  }
  e_entry = this->elf.eh.e_entry + load_bias;
  entry = e_entry;
  elf_bss += load_bias;
  elf_brk += load_bias;
  start_code += load_bias;
  end_code += load_bias;
  end_data += load_bias;
  start_data += load_bias;
  uint64_t saved_perms = this->mem.getPerms(elf_bss);
  this->mem.mprotect_l((void *)this->mem.floor(elf_bss), elf_brk - elf_bss,
                       PROT_READ | PROT_WRITE);
  for (uint64_t i = elf_bss; i < elf_brk; i++) {
    this->mem.putchar((void *)i, 0x00);
  }
  this->mem.mprotect_l((void *)this->mem.floor(elf_bss), elf_brk - elf_bss,
                       saved_perms);
  elf_program_headers =
      this->mem.mmap_l((void *)NULL, this->elf.eh.e_phnum * sizeof(elf_ph_64),
                       PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANON, 0, 0);
  if (elf_program_headers == NULL) {
    std::cout << "[ERROR] failed to allocate block" << std::endl;
    exit(0);
  }
  this->mem.write_l(elf_program_headers, (const char *)this->elf.ph,
                    this->elf.eh.e_phnum * sizeof(elf_ph_64));
  if (interpreter) {
    temp_reserve =
        this->mem.mmap_l((void *)elf_brk, 0x1000000, PROT_WRITE | PROT_READ,
                         MAP_PRIVATE | MAP_ANON, 0, 0);
    if (temp_reserve == NULL) {
      std::cout << "[ERROR] Failed to reserve brk space" << std::endl;
      exit(0);
    }
    entry = interpreter->eh.e_entry;
    end_code = 0;
    end_data = 0;
    elf_brk = 0;
    elf_bss = 0;
    load_bias = 0;
    load_addr_set = 0;
    for (auto it = interpreter->header.begin(); it != interpreter->header.end();
         ++it) {
      eph = *it;
      if (eph->p_type != PT_LOAD) {
        continue;
      }
      pageoffset = eph->p_vaddr & ((1 << this->mem.pagebits) - 1);
      flags = MAP_ANON | MAP_PRIVATE;
      vaddr = eph->p_vaddr - pageoffset;
      memsz = this->mem.ceil(eph->p_memsz + pageoffset + 1);
      offset = eph->p_offset - pageoffset;
      filesz = eph->p_filesz + pageoffset;
      prot = make_prot(eph->p_flags);
      void *r = this->mem.mmapFile((char *)load_bias + vaddr, filesz, flags,
                                   prot, offset, interpreter->fp);
      if (!r) {
        std::cout << "[ERROR] Failed to load interpreter" << std::endl;
        exit(0);
      }
      if (!load_addr_set) {
        load_addr_set = 1;
        load_bias = reinterpret_cast<uint64_t>(r);
      }
      k = load_bias + vaddr + filesz;
      if (k > elf_bss) {
        elf_bss = k;
      }
      if ((eph->p_flags & PF_X) && end_code < k) {
        end_code = k;
      }
      if (end_data < k) {
        end_data = k;
      }
      k = load_bias + vaddr + filesz;
      if (k > elf_bss) {
        elf_bss = k;
      }
      if ((eph->p_flags & PF_X) && end_code < k) {
        end_code = k;
      }
      if (end_data < k) {
        end_data = k;
      }
      k = load_bias + vaddr + memsz;
      if (k > elf_brk) {
        bss_prot = prot;
        elf_brk = k;
      }
    }
    entry += load_bias;
    this->mem.munmap_l(temp_reserve, 0x1000000);
  }
  stack_base = this->mem.mmap_l(NULL, 0x30000, PROT_READ | PROT_WRITE,
                                MAP_ANON | MAP_PRIVATE, 0, 0);
  if (stack_base == NULL) {
    std::cout << "[ERROR] Failed to allocate the stack buffer" << std::endl;
    exit(0);
  }
  char *at_random = NULL;
  char *temp_stack = (char *)this->mem.ceil(
      reinterpret_cast<uint64_t>(stack_base) + 0x30000 - 1);
  temp_stack -= 16;
  for (int i = 0; i < 16; i++) {
    this->mem.putchar(temp_stack + i, 0xde);
  }
  at_random = temp_stack;
  temp_stack -= 7;
  this->mem.write_l(temp_stack, "x86_64", 6);
  uint64_t cpu_hint = reinterpret_cast<uint64_t>(temp_stack);
  std::vector<char *> envp_list;
  std::vector<char *> argv_list;
  std::reverse(std::begin(envp), std::end(envp));
  for (auto it = envp.begin(); it != envp.end(); ++it) {
    temp_stack -= (*it).size() + 1;
    this->mem.write_l(temp_stack, (*it).c_str(), (*it).size());
    envp_list.push_back(temp_stack);
  }
  std::reverse(std::begin(envp_list), std::end(envp_list));
  temp_stack -= p.string().size() + 1;
  uint64_t fn_inmem = reinterpret_cast<uint64_t>(temp_stack);
  this->mem.write_l(temp_stack, p.c_str(), p.string().size());
  argv_list.push_back(temp_stack);
  temp_stack =
      (char *)(reinterpret_cast<uint64_t>(temp_stack) & 0xfffffffffffffff0);
  temp_stack -= sizeof(Elf64_auxv_t) * 20;
  Elf64_auxv_t *auxv = new Elf64_auxv_t[25];
  if (auxv == NULL) {
    std::cout << "[ERROR] new failed" << std::endl;
    exit(0);
  }
  load_vdso();
  auxv[19].a_type = AT_NULL;
  auxv[19].a_val = AT_NULL;
  auxv[18].a_type = AT_PLATFORM;
  auxv[18].a_val = cpu_hint;
  auxv[17].a_type = AT_EXECFN;
  auxv[17].a_val = fn_inmem;
  auxv[16].a_type = AT_HWCAP2;
  auxv[16].a_val = 0;
  auxv[15].a_type = AT_RANDOM;
  auxv[15].a_val = reinterpret_cast<uint64_t>(at_random);
  auxv[14].a_type = AT_SECURE;
  auxv[14].a_val = 0;
  auxv[13].a_type = AT_EGID;
  auxv[13].a_val = 1000;
  auxv[12].a_type = AT_GID;
  auxv[12].a_val = 1000;
  auxv[11].a_type = AT_EUID;
  auxv[11].a_val = 1000;
  auxv[10].a_type = AT_UID;
  auxv[10].a_val = 1000;
  auxv[9].a_type = AT_ENTRY;
  auxv[9].a_val = e_entry;
  auxv[8].a_type = AT_FLAGS;
  auxv[8].a_val = 0;
  auxv[7].a_type = AT_BASE;
  auxv[7].a_val = load_bias;
  auxv[6].a_type = AT_PHNUM;
  auxv[6].a_val = this->elf.eh.e_phnum;
  auxv[5].a_type = AT_PHENT;
  auxv[5].a_val = this->elf.eh.e_phentsize;
  auxv[4].a_type = AT_PHDR;
  auxv[4].a_val = reinterpret_cast<uint64_t>(elf_program_headers);
  auxv[3].a_type = AT_CLKTCK;
  auxv[3].a_val = 100;
  auxv[2].a_type = AT_PAGESZ;
  auxv[2].a_val = 0x1000;
  auxv[1].a_type = AT_HWCAP;
  auxv[1].a_val = 0x178bfbff;
  auxv[0].a_type = AT_SYSINFO_EHDR;
  auxv[0].a_val = reinterpret_cast<uint64_t>(this->vdso);
  this->mem.write_l(temp_stack, (char *)auxv, sizeof(Elf64_auxv_t) * 20);
  delete[] auxv;
  temp_stack -= 8;
  for (auto it = envp_list.rbegin(); it != envp_list.rend(); ++it) {
    temp_stack -= 8;
    this->mem.write_l(temp_stack, (char *)&(*it), sizeof(char *));
  }
  temp_stack -= 8;
  for (auto it = argv_list.rbegin(); it != argv_list.rend(); ++it) {
    temp_stack -= 8;
    this->mem.write_l(temp_stack, (char *)&(*it), sizeof(char *));
  }
  temp_stack -= 8;
  uint64_t j = argv_list.size();
  this->mem.write_l(temp_stack, (char *)&(j), sizeof(uint64_t));
  this->cpu.setreg("rax", 0);
  this->cpu.setreg("rbx", 0);
  this->cpu.setreg("rcx", 0);
  this->cpu.setreg("rdx", 0);
  this->cpu.setreg("rsp", reinterpret_cast<uint64_t>(temp_stack));
  this->cpu.setreg("rbp", 0);
  this->cpu.setreg("rsi", 0);
  this->cpu.setreg("rdi", 0);
  this->cpu.setreg("rip", reinterpret_cast<uint64_t>(entry));
  this->cpu.setreg("r8", 0);
  this->cpu.setreg("r9", 0);
  this->cpu.setreg("r10", 0);
  this->cpu.setreg("r11", 0);
  this->cpu.setreg("r12", 0);
  this->cpu.setreg("r13", 0);
  this->cpu.setreg("r14", 0);
  this->cpu.setreg("r15", 0);
  this->cpu.setreg("rflags", 0x202);
  this->cpu.setreg("cs", 0x23);
  this->cpu.setreg("ss", 0x2b);
  this->cpu.setreg("ds", 0x2b);
  this->cpu.setreg("es", 0x2b);
  this->cpu.setreg("fs", 0);
  this->cpu.setreg("gs", 0);
  free(elf_interpreter);
  os.elfbrk = elf_brk;
  os.elfbrk_allocd = elf_brk;
  os.cpu = &cpu;
  os.mem = &mem;
  os.elf = elf.elfname;
  cpu.mem = &mem;
  cpu.init_fs_segment();
  return;
}
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
int writit(char *addr, char *nm) {
  struct stat st;
  int fd;
  if (stat(nm, &st) != 0) {
    printf("[ERROR} failed to stat: %s\n", nm);
    exit(0);
  }
  fd = open(nm, O_RDONLY);
  if (fd <= 0) {
    printf("[ERROR] failed to open %s: %s\n", nm, strerror(errno));
    exit(0);
  }
  std::cout << "Reading bytes: " << (int)st.st_size << std::endl;
  read(fd, addr, st.st_size);
  close(fd);
  return 1;
}
int System::Run() {
  char cmd[128];
  uint64_t ins_count = 0;
  uint64_t break_instruction = 0;
  uint64_t break_address = 0;
  uint64_t step = 1;
  uint64_t print_ins = 1;
  uint64_t print_regs = 1;
  uint64_t print_ext = 0;
  uint64_t read_addr = 0;
  uint64_t read_value = 0;
  uint64_t l;
  std::vector<std::string> tokenized_cmd;
  std::cout << "[INFO] Executing now..." << std::hex << std::endl;
  while (1) {
    if (step) {
      std::cout << (int)ins_count << "# ";
      memset(cmd, 0, 128);
      fgets(cmd, 127, stdin);
      l = strlen(cmd);
      if (!l) {
        continue;
      }
      if (cmd[l - 1] == '\n') {
        cmd[l - 1] = 0x00;
      }
      tokenized_cmd = cpptok(std::string(cmd), ' ');
      if (tokenized_cmd[0] == "c") {
        break_instruction = -1;
        break_address = 0;
        step = 0;
      } else if (tokenized_cmd[0] == "g") {
        if (tokenized_cmd.size() != 2) {
          std::cout << "g <address>" << std::endl;
          continue;
        }
        break_address = strtoull(tokenized_cmd[1].c_str(), NULL, 16);
        break_instruction = -1;
        step = 0;
      } else if (tokenized_cmd[0] == "t") {
        if (tokenized_cmd.size() != 2) {
          std::cout << "t <skip count>" << std::endl;
          continue;
        }
        break_address = 0;
        break_instruction =
            ins_count + strtoull(tokenized_cmd[1].c_str(), NULL, 16);
        step = 0;
      } else if (tokenized_cmd[0] == "s") {
        break_address = 0;
        break_instruction = ins_count + 1;
        step = 1;
      } else if (tokenized_cmd[0] == "set") {
        if (tokenized_cmd.size() != 2) {
          std::cout << "set [pr | pe | pi]" << std::endl;
          continue;
        }
        if (tokenized_cmd[1] == "pr") {
          print_regs = 1;
        } else if (tokenized_cmd[1] == "pe") {
          print_ext = 1;
        } else if (tokenized_cmd[1] == "pi") {
          print_ins = 1;
        }
        continue;
      } else if (tokenized_cmd[0] == "unset") {
        if (tokenized_cmd.size() != 2) {
          std::cout << "unset [pr | pe | pi]" << std::endl;
          continue;
        }
        if (tokenized_cmd[1] == "pr") {
          print_regs = 0;
        } else if (tokenized_cmd[1] == "pe") {
          print_ext = 0;
        } else if (tokenized_cmd[1] == "pi") {
          print_ins = 0;
        }
        continue;
      } else if (tokenized_cmd[0] == "x") {
        if (tokenized_cmd.size() != 2) {
          std::cout << "x [<address> | <reg>]" << std::endl;
          continue;
        }
        if (cpu.isReg(tokenized_cmd[1])) {
          read_addr = cpu.getreg(tokenized_cmd[1]);
        } else {
          read_addr = strtoull(tokenized_cmd[1].c_str(), NULL, 16);
        }
        if (mem.isReadable(read_addr, 8)) {
          mem.read_l((void *)read_addr, (char *)&read_value, 8);
          std::cout << "0x" << read_addr << ":\t0x" << std::setfill('0')
                    << std::setw(16) << read_value << std::endl;
        } else {
          std::cout << "0x" << read_addr << ":\tCannot access memory"
                    << std::endl;
        }
        continue;
      } else if (tokenized_cmd[0] == "pm") {
        mem.walkTheBlock();
        continue;
      } else if (tokenized_cmd[0] == "q") {
        return 0;
      }
    }
    ins_count++;
    try {
      cpu.step(print_ins);
    } catch (SycallException e) {
      if (os.syscall_dispatch()) {
        cpu.printState();
        std::cout << "[INFO] Total instructions: " << ins_count << std::endl;
        std::cout << "[INFO] Execution Completed" << std::endl;
        int waitforit;
        std::cin >> waitforit;
        std::cerr << "[INFO] Execution Completed" << std::endl;
        return 0;
      }
    } catch (MemoryException e) {
      std::cout << e.what() << std::endl;
      cpu.printState();
      return 0;
    } catch (const FloatingPointException &e) {
      std::cout << e.what() << std::endl;
    } catch (InstructionException e) {
      std::cout << e.what() << ": " << e.error_number << std::endl;
      return 0;
    }
    if (print_regs) {
      cpu.printState();
    }
    if (print_ext) {
      cpu.printExtendedState();
    }
    if (ins_count >= break_instruction || break_address == cpu._rip.val) {
      step = 1;
    }
  }
  return 0;
}
