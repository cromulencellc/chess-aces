#include "memblock.hpp"

#include <stdio.h>

MemBlock::MemBlock( uint64_t start, uint64_t size, uint64_t perms, uint64_t pagebits)
{
    this->start = start;
    this->pagebits = pagebits;

    this->end = start + size;
    this->size = size;
    this->perms = perms;

    this->data = NULL;

    return;
}