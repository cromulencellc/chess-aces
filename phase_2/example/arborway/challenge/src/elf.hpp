#ifndef __ELF_HPP__
#define __ELF_HPP__

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>

#include <iostream>
#include <vector>

#define SZ_16M              0x01000000
#define TASK_SIZE_64    0x7fffffffe000
#define ELF_ET_DYN_BASE (2 * TASK_SIZE_64 / 3)

#define ET_NONE     0x00
#define ET_REL      0x01
#define ET_EXEC     0x02
#define ET_DYN      0x03
#define ET_CORE     0x04
#define ET_LOOS     0xfe00
#define ET_HIOS     0xfeff
#define ET_LOPROC   0xff00
#define ET_HIPROC   0xffff

#define PT_NULL     0x00000000
#define PT_LOAD     0x00000001
#define PT_DYNAMIC  0x00000002
#define PT_INTERP   0x00000003
#define PT_NOTE     0x00000004
#define PT_SHLIB    0x00000005
#define PT_PHDR     0x00000006
#define PT_TLS      0x00000007
#define PT_LOOS     0x60000000
#define PT_HIOS     0x6FFFFFFF
#define PT_LOPROC   0x70000000
#define PT_HIPROC   0x7FFFFFFF

#define PT_GNU_EH_FRAME     0x6474e550
#define PT_GNU_STACK        (PT_LOOS + 0x474e551)

#define PF_R        0x04
#define PF_W        0x02
#define PF_X        0x01

typedef struct {
  uint64_t a_type;      /* Entry type */
  uint64_t a_val;
} Elf64_auxv_t;

#define AT_NULL     0       /* End of vector */
#define AT_IGNORE   1       /* Entry should be ignored */
#define AT_EXECFD   2       /* File descriptor of program */
#define AT_PHDR     3       /* Program headers for program */
#define AT_PHENT    4       /* Size of program header entry */
#define AT_PHNUM    5       /* Number of program headers */
#define AT_PAGESZ   6       /* System page size */
#define AT_BASE     7       /* Base address of interpreter */
#define AT_FLAGS    8       /* Flags */
#define AT_ENTRY    9       /* Entry point of program */
#define AT_NOTELF   10      /* Program is not ELF */
#define AT_UID      11      /* Real uid */
#define AT_EUID     12      /* Effective uid */
#define AT_GID      13      /* Real gid */
#define AT_EGID     14      /* Effective gid */
#define AT_HWCAP    16      /* arch dependent hints at CPU capabilities */
#define AT_PLATFORM 15      /* string identifying CPU for optimizations */
#define AT_CLKTCK   17      /* Frequency of times() */

#define AT_SECURE   23      /* Boolean, was exec setuid-like?  */

#define AT_RANDOM   25      /* Address of 16 random bytes.  */
#define AT_HWCAP2   26      /* extension of AT_HWCAP */
#define AT_EXECFN   31      /* filename of program */
/* The address of a page containing the virtual Dynamic Shared
              Object (vDSO) that the kernel creates in order to provide fast
              implementations of certain system calls.*/
#define AT_SYSINFO_EHDR 33  /// TODO: add this one

typedef struct elf_header_64 {
    char e_ident[4];
    uint8_t ei_class;
    uint8_t ei_data;
    uint8_t ei_version;
    uint8_t ei_osabi;
    uint8_t ei_abiversion;
    char ei_pad[7];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} elf_header_64 __attribute__((aligned(1)));

typedef struct elf_ph_64 {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} elf_ph_64 __attribute__((aligned(1)));

typedef struct elf_sh_64 {
    char sh_name[4];
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
} elf_sh_64 __attribute__((aligned(1)));

uint32_t make_prot( uint32_t flags );

class Elf {
public:
    //int file_length;
    FILE *fp;

    elf_header_64 eh;

    // Array of program headers;
    elf_ph_64 * ph;

    std::string elfname;
    
    std::vector< elf_ph_64 *> header;
    std::vector< elf_sh_64 *> segment_header;

    int parse_elf( std::string filename );

};



#endif