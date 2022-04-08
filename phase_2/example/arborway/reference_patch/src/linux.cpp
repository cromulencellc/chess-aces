#include "linux.hpp"
Linux::Linux() {
  char c[PATH_MAX + 1];
  getcwd(c, PATH_MAX);
  cwd = std::string(c);
  valid_fds.push_back(0);
  valid_fds.push_back(1);
  valid_fds.push_back(2);
  return;
}
uint64_t Linux::syscall_dispatch() {
  uint64_t sc = cpu->getreg("rax");
  uint64_t arg0 = cpu->getreg("rdi");
  uint64_t arg1 = cpu->getreg("rsi");
  uint64_t arg2 = cpu->getreg("rdx");
  uint64_t arg3 = cpu->getreg("r10");
  uint64_t arg4 = cpu->getreg("r8");
  uint64_t arg5 = cpu->getreg("r9");
  switch (sc) {
  case 0x1:
    return sys_write(arg0, arg1, arg2);
    break;
  case 0x5:
    return sys_fstat(arg0, arg1);
    break;
  case 0x9:
    return sys_mmap(arg0, arg1, arg2, arg3, arg4, arg5);
    break;
  case 0xa:
    return sys_mprotect(arg0, arg1, arg2);
    break;
  case 0xb:
    return sys_munmap(arg0, arg1);
    break;
  case 0xc:
    return sysbrk(arg0);
    break;
  case 0x15:
    return sys_access(arg0, arg1);
    break;
  case 0x3f:
    return sys_uname(arg0);
    break;
  case 0x59:
    return sys_readlink(arg0, arg1, arg2);
    break;
  case 0x9e:
    return sys_arch_prctl(arg0, arg1);
    break;
  case 0xe7:
    return sys_exit_group(arg0);
    break;
  case 0x101:
    return sys_openat(arg0, arg1, arg2, arg3);
    break;
  default:
    std::cout << "[FAIL] Unhandled syscall: " << sc << std::endl;
    exit(0);
    break;
  };
  return 0;
}
uint64_t Linux::sys_exit_group(uint64_t status) { return 1; }
uint64_t Linux::sys_write(uint64_t fd, uint64_t buf, uint64_t count) {
  uint64_t result = 0;
  if (mem->isReadable(buf, count) == false) {
    cpu->setreg("rax", -1);
    return 0;
  }
  auto it = std::find(valid_fds.begin(), valid_fds.end(), fd);
  if (it == valid_fds.end()) {
    cpu->setreg("rax", -1);
    return 0;
  }
  result = write(fd, (const char *)buf, count);
  cpu->setreg("rax", result);
  return 0;
}
uint64_t Linux::sys_fstat(uint64_t fd, uint64_t statbuf) {
  uint64_t result = 0;
  if (mem->isWriteable(statbuf, sizeof(struct stat)) == false) {
    cpu->setreg("rax", -1);
    return 0;
  }
  auto it = std::find(valid_fds.begin(), valid_fds.end(), fd);
  if (it == valid_fds.end()) {
    cpu->setreg("rax", -1);
    return 0;
  }
  result = fstat(fd, (struct stat *)statbuf);
  cpu->setreg("rax", result);
  return 0;
}
uint64_t Linux::sys_access(uint64_t filename, uint64_t mode) {
  std::string rpath = "";
  uint64_t result = 0;
  if (mem->readstring(filename, rpath) == -1) {
    std::cout << "segfault on read at: 0x" << filename << std::endl;
    exit(0);
  }
  result = access(rpath.c_str(), mode);
  cpu->setreg("rax", result);
  return 0;
}
uint64_t Linux::sys_mprotect(uint64_t addr, uint64_t len, uint64_t prot) {
  uint64_t result = mem->mprotect_l((void *)addr, len, prot);
  cpu->setreg("rax", result);
  return 0;
}
uint64_t Linux::sys_munmap(uint64_t addr, uint64_t len) {
  uint64_t result = mem->munmap_l((void *)addr, len);
  cpu->setreg("rax", result);
  return 0;
}
uint64_t Linux::sys_mmap(uint64_t addr, uint64_t len, uint64_t prot,
                         uint64_t flags, uint64_t fd, uint64_t off) {
  void *result = mem->mmap_l((void *)addr, len, prot, flags, fd, off);
  cpu->setreg("rax", reinterpret_cast<uint64_t>(result));
  return 0;
}
uint64_t Linux::sys_arch_prctl(uint64_t code, uint64_t addr) {
  if (code != 0x1002) {
    std::cout << "[ERROR] Unknown arch code: " << std::hex << code << std::endl;
    exit(0);
  }
  cpu->_fs.val = 0;
  cpu->fs_segment = (void *)addr;
  cpu->setreg("rax", 0);
  return 0;
}
uint64_t Linux::sysbrk(uint64_t addr) {
  void *nb = NULL;
  uint64_t embiggen = 0;
  uint64_t brk_perms = 0;
  if (addr == 0) {
    cpu->setreg("rax", elfbrk);
    return 0;
  }
  if (elfbrk < addr && addr <= elfbrk_allocd) {
    elfbrk = addr;
  } else {
    brk_perms = mem->getPerms(elfbrk - 1);
    embiggen = mem->ceil(addr - elfbrk_allocd);
    nb = mem->mmap_l((void *)elfbrk_allocd, embiggen, brk_perms,
                     MAP_ANON | MAP_PRIVATE, 0, 0);
    if (nb == NULL) {
      cpu->setreg("rax", -1);
      return 0;
    }
    elfbrk_allocd = reinterpret_cast<uint64_t>(nb) + embiggen;
    elfbrk = addr;
  }
  cpu->setreg("rax", elfbrk);
  return 0;
}
uint64_t Linux::sys_openat(uint64_t dirfd, uint64_t pathname, uint64_t flags,
                           uint64_t mode) {
  int fd = 0;
  fd = openat(dirfd, (const char *)pathname, flags, mode);
  cpu->setreg("rax", fd);
  valid_fds.push_back(fd);
  return 0;
}
uint64_t Linux::sys_uname(uint64_t buf) {
  int result = 0;
  if (mem->isWriteable(buf, sizeof(struct utsname)) == false) {
    std::cerr << sizeof(struct utsname) << std::endl;
    std::cout << "segfault: 0x" << (int)buf << std::endl;
    exit(0);
  }
  result = uname((struct utsname *)buf);
  cpu->setreg("rax", result);
  return 0;
}
uint64_t Linux::sys_readlink(uint64_t pathname, uint64_t buf, uint64_t bufsiz) {
  std::string rpath = "";
  char *fp = NULL;
  ssize_t to_write = 0;
  if (mem->readstring(pathname, rpath) == -1) {
    std::cout << "segfault on read at: 0x" << pathname << std::endl;
    exit(0);
  }
  if (mem->isWriteable(buf, bufsiz) == false) {
    std::cout << "segfault on write at: 0x" << buf << std::endl;
    exit(0);
  }
  if (rpath == "/proc/self/exe") {
    fp = realpath(elf.c_str(), NULL);
    if (fp == NULL) {
      cpu->setreg("rax", -1);
      return 0;
    }
    rpath = std::string(fp);
    free(fp);
  } else {
    fp = realpath(rpath.c_str(), NULL);
    if (fp == NULL) {
      cpu->setreg("rax", -1);
      return 0;
    }
    rpath = std::string(fp);
    free(fp);
  }
  if (bufsiz < rpath.size()) {
    to_write = bufsiz;
  } else {
    to_write = rpath.size();
  }
  if (mem->write_l((void *)buf, rpath.c_str(), to_write)) {
    std::cout << "segfault writing to: 0x" << buf << std::endl;
    exit(0);
  }
  cpu->setreg("rax", rpath.size());
  return 0;
}