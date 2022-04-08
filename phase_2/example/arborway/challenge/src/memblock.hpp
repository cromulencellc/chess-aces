#ifndef __MEMBLOCK_HPP__
#define __MEMBLOCK_HPP__

#include <stdint.h>
#include <stdio.h>

#include <iostream>
#include <map>

#include "syscall_int.hpp"

/*
    This is the representation of a block of memory with a fixed start and size;
 */
class MemBlock {
  public:
    uint64_t start;
    uint64_t end;
    uint64_t size;
    uint64_t perms;

    uint64_t pagebits;

    char *data;

    MemBlock( uint64_t start, uint64_t size, uint64_t perms, uint64_t pagebits);
};



#endif