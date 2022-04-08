#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include "memory.hpp"
#include "elf.hpp"
#include "cpu.hpp"
#include "linux.hpp"

std::vector< std::string > vectorize_array( char **arr );

class System {
  public: 
    Elf elf;
    Memory mem;
    Cpu cpu;
    Linux os;
    
    std::string vdso_filename;
    char *vdso;
    unsigned int vdso_length;

    System( char *data, uint64_t length);
    System( std::string filename, std::vector<std::string> envp, std::string vdso );
    int Run( );
    void load_vdso( );
};


#endif