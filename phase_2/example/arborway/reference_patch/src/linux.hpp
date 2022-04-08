#ifndef __LINUX_HPP__
#define __LINUX_HPP__
#include "cpu.hpp"
#include "elf.hpp"
#include "memory.hpp"
#include <algorithm>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>
class Linux {
public:
  Cpu *cpu;
  Memory *mem;
  std::string cwd;
  uint64_t elfbrk_allocd;
  uint64_t elfbrk;
  std::string elf;
  std::vector<uint64_t> valid_fds;
  Linux();
  uint64_t syscall_dispatch();
  uint64_t sysbrk(uint64_t addr);
  uint64_t sys_arch_prctl(uint64_t code, uint64_t addr);
  uint64_t sys_openat(uint64_t dirfd, uint64_t pathname, uint64_t flags,
                      uint64_t mode);
  uint64_t sys_uname(uint64_t buf);
  uint64_t sys_readlink(uint64_t pathname, uint64_t buf, uint64_t bufsiz);
  uint64_t sys_mmap(uint64_t addr, uint64_t len, uint64_t prot, uint64_t flags,
                    uint64_t fd, uint64_t off);
  uint64_t sys_munmap(uint64_t addr, uint64_t len);
  uint64_t sys_mprotect(uint64_t addr, uint64_t len, uint64_t prot);
  uint64_t sys_access(uint64_t filename, uint64_t mode);
  uint64_t sys_fstat(uint64_t fd, uint64_t statbuf);
  uint64_t sys_write(uint64_t fd, uint64_t buf, uint64_t count);
  uint64_t sys_exit_group(uint64_t status);
  uint64_t read_proc_data(std::string procfile, char *data, uint64_t size);
};
#endif