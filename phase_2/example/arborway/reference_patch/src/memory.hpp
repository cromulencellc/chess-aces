#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__
#include <algorithm>
#include <cassert>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include "memblock.hpp"
#include "syscall_int.hpp"
#include "uint256_t.h"
class Memory {
public:
  uint32_t pagebits;
  std::map<uint64_t, MemBlock *> page_to_memblock;
  uint64_t floor(uint64_t addr);
  uint64_t ceil(uint64_t addr);
  uint64_t page(uint64_t addr);
  uint64_t find_empty_block(uint64_t start, uint64_t size);
  MemBlock *getBlock(uint64_t addr);
  uint64_t getPerms(uint64_t addr);
  bool isWriteable(uint64_t addr, uint64_t size);
  bool isReadable(uint64_t addr, uint64_t size);
  int putchar(void *addr, uint8_t c);
  int write_l(void *addr, const char *data, uint64_t length);
  int read_l(void *addr, char *dest, uint64_t size);
  int writeymm(void *addr, uint256_t value, uint64_t length);
  void walkTheBlock();
  int readint(void *addr, uint64_t *dest, uint64_t size);
  int readint(void *addr, uint256_t *dest, uint64_t size);
  int readstring(uint64_t addr, std::string &out);
  Memory();
  void *mmap_l(void *addr, uint64_t size, uint64_t perms, uint32_t flags,
               uint64_t fd, uint64_t offset);
  int munmap_l(void *addr, uint64_t size);
  void *mmapFile(void *addr, uint64_t size, uint32_t flags, uint32_t perms,
                 uint64_t offset, FILE *fp);
  int mprotect_l(void *addr, uint64_t size, uint64_t perms);
};
#endif
