#include "elf.hpp"

#include <errno.h>

int Elf::parse_elf( std::string filename )
{
    int result = 0;
    FILE *fp = NULL;
    size_t r;
    elf_sh_64 * esh = NULL;

    fp = fopen( filename.c_str(), "r");

    if ( fp == NULL ) {
        std::cout << "[ERROR] Failed to open: " << filename << " " << strerror( errno ) << std::endl;

        return result;
    }

    elfname = filename;
    
    r = fread(&(this->eh), 1, sizeof(struct elf_header_64), fp);

    if ( r != sizeof(struct elf_header_64) ) {
        std::cout << "[ERROR] Failed to read the ELF header: " << r << std::endl;

        fclose(fp);

        return result;
    }

    if( memcmp( this->eh.e_ident, "\x7f"  "ELF", 4) != 0) {
        std::cout << "[ERROR] Invalid ELF magic" << std::endl;
        fclose(fp);

        return result;
    }

    if ( this->eh.ei_class != 2 ) {
        std::cout << "[ERROR] 64-bit only" << std::endl;

        fclose(fp);

        return result;
    }

    if ( this->eh.ei_data != 1 ) {
        std::cout << "[ERROR] Little endian only" << std::endl;

        fclose(fp);

        return result;
    }

    if ( this->eh.ei_version != 1 ) {
        std::cout << "[ERROR] Incorrect ELF version" << std::endl;

        fclose(fp);

        return result;
    }

    if ( this->eh.ei_osabi != 0 && this->eh.ei_osabi != 3) {
        std::cout << "[ERROR] System-V or UNIX-GNU only: " << (int)this->eh.ei_osabi << std::endl;

        fclose(fp);

        return result;
    }

    if ( this->eh.ei_abiversion != 0 ) {
        std::cout << "[ERROR] Invalid ABI version" << std::endl;

        fclose(fp);

        return result;
    }


    if ( this->eh.e_type != ET_EXEC ) { //} && this->eh.e_type != ET_DYN) {
        std::cout << "[ERROR] Invalid object file type" << std::endl;

        fclose(fp);

        return result;
    }

    if ( this->eh.e_machine != 0x3e ) {
        std::cout << "[ERROR] Invalid target machine: " << std::to_string(this->eh.e_machine) << std::endl;

        fclose(fp);

        return result;
    }

    if ( this->eh.e_ehsize != sizeof(struct elf_header_64) ) {
        std::cout << "[ERROR] Invalid elf header size: " << this->eh.e_ehsize << std::endl;

        fclose(fp);

        return result;
    }

    if ( fseek( fp, this->eh.e_phoff, SEEK_SET) != 0 ) {
        std::cout << "[ERROR] Failed to seek to program header offset" << std::endl;

        fclose(fp);

        return result;
    }

    this->ph = new elf_ph_64[this->eh.e_phnum + 1];

    if ( this->ph == NULL ) {
        std::cout << "[ERROR] Memory allocation error" << std::endl;
        fclose(fp);

        return result; 
    }

    r = fread(this->ph, sizeof(struct elf_ph_64), this->eh.e_phnum, fp);

    if ( r != this->eh.e_phnum ) {
        std::cout << "[ERROR] Failed to read the ELF program header: " << r << std::endl;

        fclose(fp);

        return result;
    }

    for ( int i = 0; i < this->eh.e_phnum; i++) {
        this->header.push_back(&this->ph[i]);

    }

    if ( fseek( fp, this->eh.e_shoff, SEEK_SET) != 0 ) {
        std::cout << "[ERROR] Failed to seek to section header offset" << std::endl;

        fclose(fp);

        return result;
    }

    /// /// https://elixir.bootlin.com/linux/latest/source/fs/binfmt_elf.c#L728
    for ( int i = 0; i < this->eh.e_shnum; i++) {
        esh = new elf_sh_64;

        if ( esh == NULL ) {
            std::cout << "[ERROR] Memory allocation error" << std::endl;

            /// TODO: free all the allocated blocks but perhaps in the destructor
            fclose(fp);

            return result;
        }

        r = fread(esh, 1, sizeof(struct elf_sh_64), fp);

        if ( r != sizeof(struct elf_sh_64) ) {
            std::cout << "[ERROR] Failed to read the ELF segment header: " << r << std::endl;

            fclose(fp);

            return result;
        }
        
        this->segment_header.push_back(esh);

    }

    this->fp = fp;

    result = 1;

    return result;
}