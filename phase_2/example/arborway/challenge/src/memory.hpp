#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__

#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <algorithm>
#include <cassert>

#include "memblock.hpp"
#include "uint256_t.h"
#include "syscall_int.hpp"

/** class Memory
 * Parent class for all memory management operations
 */
class Memory {
  public:
    uint32_t pagebits;

    std::map<uint64_t, MemBlock *> page_to_memblock;

    uint64_t floor( uint64_t addr );
    uint64_t ceil( uint64_t addr );
    uint64_t page( uint64_t addr);
    uint64_t find_empty_block( uint64_t start, uint64_t size);
    MemBlock *getBlock( uint64_t addr);
    uint64_t getPerms( uint64_t addr );

    bool isWriteable( uint64_t addr, uint64_t size );
    bool isReadable( uint64_t addr, uint64_t size );

    int putchar( void *addr, uint8_t c);
    int write_l( void *addr, const char *data, uint64_t length);
    int read_l( void *addr, char *dest, uint64_t size);

    int writeymm( void *addr, uint256_t value, uint64_t length);

    void walkTheBlock( );
    
    /// This will only read 1, 2, 4 or 8 bytes
    /// Returns -1 on failure or segfault
    int readint( void *addr, uint64_t *dest, uint64_t size);

    // This will read 16 or 32 bytes
    int readint( void *addr, uint256_t *dest, uint64_t size);
    
    /// This function reads a string in memory until it reaches a null byte
    int readstring( uint64_t addr, std::string &out);

    Memory( );
    void *mmap_l( void *addr, uint64_t size, uint64_t perms, uint32_t flags, uint64_t fd, uint64_t offset );
    int munmap_l( void *addr, uint64_t size );
    void *mmapFile( void *addr, uint64_t size, uint32_t flags, uint32_t perms, uint64_t offset, FILE *fp );

    int mprotect_l( void *addr, uint64_t size, uint64_t perms);
};

#endif
