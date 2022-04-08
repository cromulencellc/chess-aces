#include "system.hpp"
#include <stdio.h>
#include <algorithm>
#include <iterator>
#include <filesystem>
#include <iomanip>

#include "syscall_int.hpp"

std::vector< std::string > vectorize_array( char **arr )
{
    std::vector<std::string> varr;

    if ( arr == NULL ) {
        return varr;
    }

    for ( int i = 0; arr[i] != NULL; i++) {
        varr.push_back( std::string(arr[i]) );
    }

    return varr;
}

///
void System::load_vdso( )
{
    char *temp_vdso = NULL;
    FILE *fp = NULL;
    uint64_t size;

    fp = fopen(vdso_filename.c_str(), "r");

    if ( fp == NULL ) {
        std::cout << "[ERROR] failed to load: " << vdso_filename << std::endl;
        exit(0);
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    //void *addr, uint64_t size, uint32_t flags, uint32_t perms, uint64_t offset, FILE *fp
    temp_vdso = (char*)mem.mmapFile( NULL, size, MAP_ANON | MAP_PRIVATE, PROT_READ | PROT_WRITE | PROT_EXEC, 0, fp);

    if ( temp_vdso == NULL ) {
        std::cout << "[ERROR] failed to load a file" << std::endl;
        exit(0);
    }

    mem.mprotect_l( temp_vdso, size, PROT_READ | PROT_EXEC);

    vdso = temp_vdso;
    vdso_length = size;

    return;
}

System::System( char *data, uint64_t size )
{ 
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

    shellcode = (char*)mem.mmap_l( (void*)0x41410000, mem.ceil(size), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, 0, 0);

    if ( shellcode == NULL ) {
        std::cerr << "[ERROR] Failed to allocate memory" << std::endl;
        exit(0);
    }

    /// Get the shellcode in the process space of the emulated data
    mem.write_l( shellcode, data, size );

    /// We need a stack
    stack = (char*)mem.mmap_l( (void*)NULL, 0x10000, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);

    if ( stack == NULL ) {
        std::cerr << "[ERROR] Failed to allocate memory" << std::endl;
        exit(0);
    }

    /// Setup the stack
    stack = stack + (0x10000 - 16);
    cpu.setreg( "rsp", reinterpret_cast<uint64_t>(stack));
    cpu.setreg( "rip", reinterpret_cast<uint64_t>(shellcode));

    this->cpu.setreg( "rax", 0);
    this->cpu.setreg( "rbx", 0);
    this->cpu.setreg( "rcx", 0);
    this->cpu.setreg( "rdx", 0);
    this->cpu.setreg( "rbp", 0);
    this->cpu.setreg( "rsi", 0);
    this->cpu.setreg( "rdi", 0);
    this->cpu.setreg( "r8", 0);
    this->cpu.setreg( "r9", 0);
    this->cpu.setreg( "r10", 0);
    this->cpu.setreg( "r11", 0);
    this->cpu.setreg( "r12", 0);
    this->cpu.setreg( "r13", 0);
    this->cpu.setreg( "r14", 0);
    this->cpu.setreg( "r15", 0);

    os.cpu = &cpu;
    os.mem = &mem;
    os.elf = "/dev/null";

    // Initialize cpu
    cpu.mem = &mem;

    cpu.init_fs_segment();

    std::cout << "[INFO] Executing now..." << std::endl;

    while(1) {
        if ( step ) {
            std::cout << (int)ins_count << "# ";

            memset( cmd, 0, 128);
            fgets(cmd, 127, stdin);

            l = strlen(cmd);

            if ( !l ) {
                continue;
            }

            if ( cmd[l-1] == '\n') {
                cmd[l-1] = 0x00;
            }

            tokenized_cmd = cpptok( std::string(cmd), ' ');

            /// The 'c' command just continues execution to the end
            if ( tokenized_cmd[0] == "c") {
                break_instruction = -1;
                break_address = 0;
                step = 0;
            } else if (tokenized_cmd[0] == "g") {
                if ( tokenized_cmd.size() != 2 ) {
                    std::cout << "g <address>" << std::endl;
                    continue;
                }

                break_address = strtoull(tokenized_cmd[1].c_str(), NULL, 16);
                break_instruction = -1;
                step = 0;
            } else if (tokenized_cmd[0] == "t") {
                if ( tokenized_cmd.size() != 2 ) {
                    std::cout << "t <skip count>" << std::endl;
                    continue;
                }

                break_address = 0;
                break_instruction = ins_count + strtoull(tokenized_cmd[1].c_str(), NULL, 16);
                step = 0;
            } else if (tokenized_cmd[0] == "s") {
                break_address = 0;
                break_instruction = ins_count + 1;
                step = 1;
            } else if ( tokenized_cmd[0] == "set") {
                if ( tokenized_cmd.size() != 2 ) {
                    std::cout << "set [pr | pe | pi]" << std::endl;
                    continue;
                }

                if ( tokenized_cmd[1] == "pr") {
                    print_regs = 1;
                } else if ( tokenized_cmd[1] == "pe") {
                    print_ext = 1;
                } else if ( tokenized_cmd[1] == "pi") {
                    print_ins = 1;
                }

                continue;
            } else if ( tokenized_cmd[0] == "unset") {
                if ( tokenized_cmd.size() != 2 ) {
                    std::cout << "unset [pr | pe | pi]" << std::endl;
                    continue;
                }

                if ( tokenized_cmd[1] == "pr") {
                    print_regs = 0;
                } else if ( tokenized_cmd[1] == "pe") {
                    print_ext = 0;
                } else if ( tokenized_cmd[1] == "pi") {
                    print_ins = 0;
                }

                continue;
            } else if (tokenized_cmd[0] == "x") {
                if ( tokenized_cmd.size() != 2 ) {
                    std::cout << "x [<address> | <reg>]" << std::endl;
                    continue;
                }

                if ( cpu.isReg( tokenized_cmd[1]) ) {
                    read_addr = cpu.getreg( tokenized_cmd[1] );
                } else {
                    read_addr = strtoull(tokenized_cmd[1].c_str(), NULL, 16);
                }

                if (mem.isReadable( read_addr, 8) ) {
                    mem.read_l( (void*)read_addr, (char*)&read_value, 8);

                    std::cout << "0x" << read_addr << ":\t0x" << std::setfill('0') << std::setw(16) << read_value << std::endl;
                } else {
                    std::cout << "0x" << read_addr << ":\tCannot access memory" << std::endl;
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

        } catch ( SycallException e ) {
            if ( os.syscall_dispatch() ) {
                cpu.printState();
                std::cout << "[INFO] Total instructions: " << ins_count << std::endl;
                std::cout << "[INFO] Execution Completed" << std::endl;
                int waitforit;
                std::cin >> waitforit;
                std::cerr << "[INFO] Execution Completed" << std::endl;
                return;
            }

        } catch ( MemoryException e ) {
            std::cout << e.what() << ": " << e.error_number << std::endl;
            cpu.printState();
            return;
        } catch ( FloatingPointException e) {
            // CWE-703: Improper Check or Handling of Exceptional Conditions
            // CWE-755: Improper Handling of Exceptional Conditions
            // CWE-209: Generation of Error Message Containing Sensitive Information
            // CWE-390: Detection of Error Condition Without Action
            std::cout << e.what() << ": " << e.error_number << std::endl;
        } catch ( InstructionException e) {
            std::cout << e.what() << ": " << e.error_number << std::endl;
            return;
        }

        if ( print_regs ) {
            cpu.printState();
        }

        if ( print_ext ) {
            cpu.printExtendedState();
        }

        if ( ins_count >= break_instruction || break_address == cpu._rip.val) {
            step = 1;
        }
    }

    return;
}

/// static int load_elf_binary(struct linux_binprm *bprm) 
/// https://elixir.bootlin.com/linux/latest/source/fs/binfmt_elf.c#L684
System::System( std::string filename, std::vector<std::string> envp, std::string vdso )
{
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

    void * stack_base;

    this->mem.pagebits = 12;

    vdso_filename = vdso;

    p = std::filesystem::absolute( std::filesystem::path(filename));

    if ( this->elf.parse_elf( filename ) == 0 ) {
        std::cout << "[FAIL] exitting..." << std::endl;

        exit(0);
    }

    /// Determine if there is an interpreter
    for ( auto it = this->elf.header.begin(); it != this->elf.header.end(); ++it) {
        eph = *it;

        if ( eph->p_type != PT_INTERP ) {
            continue;
        }

        elf_interpreter = (char*)calloc( 1, eph->p_filesz );

        if ( elf_interpreter ) {
            fseek( this->elf.fp, eph->p_offset, SEEK_SET);

            fread(elf_interpreter, 1, eph->p_filesz, this->elf.fp);
        }
        
        break;
    }

    if (elf_interpreter) {
        interpreter = new Elf( );

        if ( interpreter->parse_elf( std::string(elf_interpreter)) == 0) {
            std::cout << "[FAIL] Failed to load the interpreter" << std::endl;

            exit(0);
        }
    }

    /// Check for an executable stack
    for ( auto it = this->elf.header.begin(); it != this->elf.header.end(); ++it ) {
        eph = *it;

        if ( eph->p_type == PT_GNU_STACK) {
            if ( eph->p_flags & PF_X) {
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

    /// Time to load the segments
    for ( auto it = this->elf.header.begin(); it != this->elf.header.end(); ++it ) {
        eph = *it;

        /// If we don't have to load anything then skip it for now
        if ( eph->p_type != PT_LOAD ) {
            continue;
        }

        /* The reason for the page offset is to allow for the correct loading address of the data
            Let's say that the virtual address of a segment is 0x20dd70 and the offset in the file is at 0xdd70. 
            The start address of the page where it is located is 0x20d000. The mmap syscall allocates on page 
            boundaries so if I requested a page then the address would start at 0x1000 for example. If I then 
            just began reading at offset 0xd70 in the file then the loaded offset would be misaligned and the 
            virtual address would be invalid.

            So what has to be done is take the virtual address and find its offset within the page. In this case
            it is 0xd70. Now, everything else must be offset by this value to ensure proper alignment. Even though
            the segment begins at offset 0xdd70 in the file we actually begin reading at 0xd000.

            The memsz value needs to be page aligned but also has to account for the page offset.
        */
        pageoffset = eph->p_vaddr & ((1 << this->mem.pagebits) - 1);

        flags = MAP_PRIVATE | MAP_ANON;

        /// Pull out the virtual address of the segment but make sure to align it to the page
        vaddr = eph->p_vaddr - pageoffset;
        memsz = this->mem.ceil( eph->p_memsz + pageoffset + 1);

        // Subtract the page offset because we want to read in the entire page
        offset = eph->p_offset - pageoffset;

        // Just like p_memsz, it only accounts for the size of the header data itself, it does not
        //  account for the offset
        filesz = eph->p_filesz + pageoffset;


        /* Notes for me
            https://www.cs-fundamentals.com/c-programming/memory-layout-of-c-program-code-data-segments
            There are 3 types of object files created by a compiler:
            - Relocatable object file - A static library file. Linkers take this code and combine it with other object
                files at compile time to create an executable file.
            - Executable object fie - They are executable files that ocntain binary code and data that can be copied
                directly into memory and executed.
            - Shared object file - This is a special type of relocatable object file which is loaded into memory and
                linked dynamically at either load time or run time.

            There are generically 5 types of segments:
            - Text or Code Segment
                Contains the code for the compiled program.
            - Initialized Data Segments
                Stores gloabal, static, constant, and external variables that are already initialized.
            - Unitialized Data Segments
                Also called .bss stores all uninitialized global, static, and extern variables. It doesn't
                actually occupy any space in the object file, it is just a place holder.
            - Stack Segment
            - Heap Segment

            high address    _______________
                           |               |\
                           |               | } Command-line arguments and environment variables
                           |_______________|/
                           |     stack     |
                           |- - - -|- - - -|
                           |       |       |
                           |       v       |
                           |               |
                           |       ^       |
                           |       |       |
                           |_ _ _ _|_ _ _ _| 
                           |     heap      |
                           |_______________|
                           |     bss       |
                           |_______________|
                           |  init'd data  |
                           |_______________|
                           |    text       |
            low address    |_______________|
                
        */

        /*
        std::cout << "memsz: " << std::hex << eph->p_memsz << " page offset: " << pageoffset << " final: " << memsz << std::endl;
        std::cout << "offset: " << std::hex << eph->p_offset << " page offset: " << pageoffset << " final: " << offset << std::endl;
        std::cout << "filesz: " << std::hex << eph->p_filesz << " page offset: " << pageoffset << " final: " << filesz << std::endl;
        std::cout << "vaddr: " << std::hex << eph->p_vaddr << " page offset: " << pageoffset << " final: " << vaddr << std::endl;
        */

        /// The ET_EXEC type needs to be mapped at a fixed address
        /// If we have already set the load address then we can consider the address fixed
        if ( this->elf.eh.e_type == ET_EXEC || load_addr_set ) {
            flags |= MAP_FIXED;
        } else if ( this->elf.eh.e_type == ET_DYN ) {
            if ( interpreter ) {
                load_bias = ELF_ET_DYN_BASE;   /// 0x555555554000
            } else {
                load_bias = 0;
            }

            //std::cout << "LOAD BIAS before floor: " << std::hex << load_bias << std::endl;
            // TODO This could be a bug or issue later
            load_bias = this->mem.floor( load_bias - vaddr );
            //std::cout << "LOAD BIAS after floor: " << std::hex << load_bias << std::endl;

        }

        prot = make_prot( eph->p_flags);

        //std::cout << "Load bias: " << std::hex << load_bias << std::endl;

        //std::cout << "memsz: " << memsz << " filesz: " << filesz << std::endl;
        void *r = this->mem.mmapFile( ((char *)load_bias) + vaddr, memsz, flags, prot, offset, this->elf.fp );

        //std::cout << "REALADDR: " << std::hex << r << std::endl;
        if ( !load_addr_set ) {
            load_addr_set = 1;

            if ( this->elf.eh.e_type == ET_DYN ) {
                load_bias = vaddr - offset;

                load_bias += reinterpret_cast<uint64_t>(r);
            }

        }

        k = vaddr;

        // Determine the start of the code segment
        // Initally start code is set to the highest possible address.
        if ( ( eph->p_flags & PF_X) && k < start_code ) {
            //std::cout << "[INFO] This segment is executable set start_code to: " << std::hex << k << std::endl;
            start_code = k;
        }

        if ( start_data < k ) {
            start_data = k;
        }

        // TODO I will probably have to do some checks here

        /// Set k to the end of this segment
        k = vaddr + filesz;

        // Start setting the bss data segment location.
        /// It begins after all of the segments including the data segment
        if ( k > elf_bss) {
            elf_bss = k;
        }

        /// If this segment is executable 
        if ( ( eph->p_flags & PF_X) && end_code < k ) {
            //std::cout << "[INFO] This segment is executable set end_code to: " << std::hex << k << std::endl;
            end_code = k;
        }

        if ( end_data < k ) {
            end_data = k;
        }

        k = vaddr + memsz;

        // brk is the heap region. It should begin after the bss section
        if ( k > elf_brk ) {
            bss_prot = prot;
            elf_brk = k;

            //std::cout << "[INFO] Setting brk to: " << std::hex << k << std::endl;
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

    /// Time to map in the brk
    /*
    std::cout << "e_entry: " << std::hex << e_entry << std::endl;
    std::cout << "bss: " << std::hex << elf_bss << std::endl;
    std::cout << "brk: " << std::hex << elf_brk << std::endl;
    std::cout << "code: " << std::hex << start_code << " : " << end_code << std::endl;
    std::cout << "data: " << std::hex << start_data << " : " << end_data << std::endl;

    std::cout << "I need to zero from: " << std::hex << elf_bss << " to " << elf_brk << std::endl;
    */
    uint64_t saved_perms = this->mem.getPerms( elf_bss );

    this->mem.mprotect_l( (void*)this->mem.floor( elf_bss ), elf_brk - elf_bss, PROT_READ | PROT_WRITE );

    for ( uint64_t i = elf_bss; i < elf_brk; i++) {
        this->mem.putchar( (void *)i, 0x00 );
    }

    this->mem.mprotect_l( (void*)this->mem.floor( elf_bss ), elf_brk - elf_bss, saved_perms);

    // Load the elf program header into the program memory space
    elf_program_headers = this->mem.mmap_l( (void *)NULL, this->elf.eh.e_phnum * sizeof(elf_ph_64), PROT_WRITE | PROT_READ,  MAP_PRIVATE | MAP_ANON, 0, 0 );
    
    if ( elf_program_headers == NULL ) {
        std::cout << "[ERROR] failed to allocate block" << std::endl;
        exit(0);
    }

    this->mem.write_l( elf_program_headers, (const char *)this->elf.ph,  this->elf.eh.e_phnum * sizeof(elf_ph_64));

    // Time to load the interpreter
    if ( interpreter ) {
        //std::cout << "ELF: " << elf_interpreter << std::endl;

        //std::cout << "Requested brk: " << std::hex << elf_brk << std::endl;
        // Reserve some memory to ensure that there is a large block for the brk
        temp_reserve = this->mem.mmap_l( (void *)elf_brk, 0x1000000, PROT_WRITE | PROT_READ,  MAP_PRIVATE | MAP_ANON, 0, 0 );

        if ( temp_reserve == NULL) {
            std::cout << "[ERROR] Failed to reserve brk space" << std::endl;
            ///TODO handle error

            exit(0);
        }

        //std::cout << "Received brk: " << std::hex << temp_reserve << std::endl;

        entry = interpreter->eh.e_entry;
        end_code = 0;
        end_data = 0;
        elf_brk = 0;
        elf_bss = 0;
        load_bias = 0;
        load_addr_set = 0;

        /// Load the interpreter segments
        for ( auto it = interpreter->header.begin(); it != interpreter->header.end(); ++it ) {
            eph = *it;

            /// If we don't have to load anything then skip it for now
            if ( eph->p_type != PT_LOAD ) {
                continue;
            }

            pageoffset = eph->p_vaddr & ((1 << this->mem.pagebits) - 1);

            flags = MAP_ANON | MAP_PRIVATE;

            /// Pull out the virtual address of the segment but make sure to align it to the page
            vaddr = eph->p_vaddr - pageoffset;
            memsz = this->mem.ceil( eph->p_memsz + pageoffset + 1);
            offset = eph->p_offset - pageoffset;
            filesz = eph->p_filesz + pageoffset;

            /*
            std::cout << "memsz: " << std::hex << eph->p_memsz << " page offset: " << pageoffset << " final: " << memsz << std::endl;
            std::cout << "offset: " << std::hex << eph->p_offset << " page offset: " << pageoffset << " final: " << offset << std::endl;
            std::cout << "filesz: " << std::hex << eph->p_filesz << " page offset: " << pageoffset << " final: " << filesz << std::endl;
            std::cout << "vaddr: " << std::hex << eph->p_vaddr << " page offset: " << pageoffset << " final: " << vaddr << std::endl;
            */
            prot = make_prot( eph->p_flags);

            //std::cout << "Interpreter Load bias: " << std::hex << load_bias << std::endl;

            void *r = this->mem.mmapFile( (char *)load_bias + vaddr, filesz, flags, prot, offset, interpreter->fp );

            /// TODO better error
            if ( !r ) {
                std::cout << "[ERROR] Failed to load interpreter" << std::endl;
                exit(0);
            }

            if ( !load_addr_set ) {
                load_addr_set = 1;

                load_bias = reinterpret_cast<uint64_t>(r);
            }

            //std::cout << "Load bias: " << std::hex << load_bias << std::endl;

            /// https://elixir.bootlin.com/linux/latest/source/fs/binfmt_elf.c#L634
            k = load_bias + vaddr + filesz;

            if ( k > elf_bss) {
                elf_bss = k;
            }

            // Determine the start of the code segment
            // Initally start code is set to the highest possible address.
            if ( ( eph->p_flags & PF_X) && end_code < k ) {
                //std::cout << "[INFO] This segment is executable set start_code to: " << std::hex << k << std::endl;
                end_code = k;
            }

            if ( end_data < k ) {
                end_data = k;
            }

            // TODO I will probably have to do some checks here

            /// Set k to the end of this segment
            k = load_bias + vaddr + filesz;

            // Start setting the bss data segment location.
            /// It begins after all of the segments including the data segment
            if ( k > elf_bss) {
                elf_bss = k;
            }

            /// If this segment is executable 
            if ( ( eph->p_flags & PF_X) && end_code < k ) {
                //std::cout << "[INFO] This segment is executable set end_code to: " << std::hex << k << std::endl;
                end_code = k;
            }

            if ( end_data < k ) {
                end_data = k;
            }

            k = load_bias + vaddr + memsz;

            // brk is the heap region. It should begin after the bss section
            if ( k > elf_brk ) {
                bss_prot = prot;
                elf_brk = k;

                //std::cout << "[INFO] Setting brk to: " << std::hex << k << std::endl;
            }
        }

        entry += load_bias;

        //std::cout << "interpreter entry: " << std::hex << entry << std::endl;

        this->mem.munmap_l( temp_reserve, 0x1000000 );

    }

    stack_base = this->mem.mmap_l( NULL, 0x30000, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, 0, 0);

    /// Todo: actually handle this gracefully
    if ( stack_base == NULL ) {
        std::cout << "[ERROR] Failed to allocate the stack buffer" << std::endl;
        exit(0);
    }

    //std::cout << "Setting up argv, envp, and auxv" << std::endl;

    char *at_random = NULL;
    char *temp_stack = (char*)this->mem.ceil(reinterpret_cast<uint64_t>(stack_base) + 0x30000 - 1);

    //std::cout << "Stack_end: " << std::hex << reinterpret_cast<uint64_t>(temp_stack) << std::endl;

    // TODO actual add random
    temp_stack -= 16;

    for ( int i = 0; i < 16; i++) {
        this->mem.putchar( temp_stack+i, 0xde);
    }

    at_random = temp_stack;
    
    /// Add the cpu hint at the end of this since we don't need the whole thing
    temp_stack -= 7;
    this->mem.write_l( temp_stack, "x86_64", 6);

    uint64_t cpu_hint = reinterpret_cast<uint64_t>(temp_stack);

    std::vector< char * > envp_list;
    std::vector< char * > argv_list;

    std::reverse(std::begin(envp), std::end(envp));

    for ( auto it = envp.begin(); it != envp.end(); ++it) {
        temp_stack -= (*it).size() + 1;

        /// TODO check for segfaults
        this->mem.write_l( temp_stack, (*it).c_str(), (*it).size());

        envp_list.push_back( temp_stack );
    }

    std::reverse(std::begin(envp_list), std::end(envp_list));

    /// Handle arg0
    temp_stack -= p.string().size() + 1;

    /// COpy of the filename location in process memory space for the auxv array
    uint64_t fn_inmem = reinterpret_cast<uint64_t>(temp_stack);

    this->mem.write_l( temp_stack, p.c_str(), p.string().size());

    argv_list.push_back(temp_stack);

    // align to 16 bytes
    temp_stack = (char *)(reinterpret_cast<uint64_t>(temp_stack) & 0xfffffffffffffff0);

    temp_stack -= sizeof(Elf64_auxv_t) * 20;
    
    // https://elixir.bootlin.com/linux/latest/source/fs/binfmt_elf.c#L248
    /// Now add the auxv values
    Elf64_auxv_t * auxv = new Elf64_auxv_t[25];

    /// TODO actually handle it
    if ( auxv == NULL ) {
        std::cout << "[ERROR] new failed" << std::endl;
        exit(0);
    }

    /// load the vdso image
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

    this->mem.write_l( temp_stack, (char *)auxv, sizeof(Elf64_auxv_t) * 20);

    //std::cout << "phdr: " << reinterpret_cast<uint64_t>(elf_program_headers) << std::endl;
    //std::cout << "ee: " << e_entry << std::endl;

    delete[] auxv;

    /// NULL envp
    temp_stack -= 8;

    for ( auto it = envp_list.rbegin(); it != envp_list.rend(); ++it) {
        temp_stack -= 8;

        this->mem.write_l( temp_stack, (char *)&(*it), sizeof(char *));
    }

    // NULL argv
    temp_stack -= 8;

    for ( auto it = argv_list.rbegin(); it != argv_list.rend(); ++it) {
        temp_stack -= 8;

        this->mem.write_l( temp_stack, (char *)&(*it), sizeof(char *));
    }

    /// argc
    temp_stack -= 8;
    uint64_t j = argv_list.size();

    this->mem.write_l( temp_stack, (char *)&(j), sizeof(uint64_t));

    //std::cout << "Stack start: 0x" << std::hex << reinterpret_cast<uint64_t>(temp_stack) << std::endl;
    //std::cout << "interpreter entry: 0x" << entry << std::endl;
    //std::cout << "elf entry: 0x" << std::hex << e_entry << std::endl;

    //this->cpu.setreg( "rax", 0);
    //this->cpu.setreg( "rbx", 0);
    //this->cpu.setreg( "rcx", 0);
    //this->cpu.setreg( "rdx", 0);
    this->cpu.setreg( "rsp", reinterpret_cast<uint64_t>(temp_stack));
    //this->cpu.setreg( "rbp", 0);
    //this->cpu.setreg( "rsi", 0);
    //this->cpu.setreg( "rdi", 0);
    this->cpu.setreg( "rip", reinterpret_cast<uint64_t>(entry));
    //this->cpu.setreg( "r8", 0);
    //this->cpu.setreg( "r9", 0);
    //this->cpu.setreg( "r10", 0);
    //this->cpu.setreg( "r11", 0);
    //this->cpu.setreg( "r12", 0);
    #ifdef PATCHED_1
    this->cpu.setreg( "r13", 0);
    #endif
    this->cpu.setreg( "r14", 0);
    this->cpu.setreg( "r15", 0);

    this->cpu.setreg( "rflags", 0x202);
    this->cpu.setreg( "cs", 0x23);
    this->cpu.setreg( "ss", 0x2b);
    this->cpu.setreg( "ds", 0x2b);
    this->cpu.setreg( "es", 0x2b);
    this->cpu.setreg( "fs", 0);
    this->cpu.setreg( "gs", 0);


    free(elf_interpreter);

    // Initialize os
    os.elfbrk = elf_brk;
    os.elfbrk_allocd = elf_brk;
    os.cpu = &cpu;
    os.mem = &mem;
    os.elf = elf.elfname;

    // Initialize cpu
    cpu.mem = &mem;

    cpu.init_fs_segment();

    return;
}

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int writit( char *addr, char *nm)
{
        struct stat st;
        int fd;

        if ( stat(nm, &st) != 0 ) {
                printf("[ERROR} failed to stat: %s\n", nm);
                exit(0);
        }

        fd = open( nm, O_RDONLY);

        if ( fd <= 0 ) {
                printf("[ERROR] failed to open %s: %s\n", nm, strerror(errno));
                exit(0);
        }

        std::cout << "Reading bytes: " << (int)st.st_size << std::endl;
        read( fd, addr, st.st_size);

        close(fd);

        return 1;
}

// TODO: Check div by 0
int System::Run( )
{ 
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

    //writit((char*)cpu._rip.val, (char*)"/home/vagrant/test");

    while(1) {
        if ( step ) {
            std::cout << (int)ins_count << "# ";

            memset( cmd, 0, 128);
            fgets(cmd, 127, stdin);

            l = strlen(cmd);

            if ( !l ) {
                continue;
            }

            if ( cmd[l-1] == '\n') {
                cmd[l-1] = 0x00;
            }

            tokenized_cmd = cpptok( std::string(cmd), ' ');

            /// The 'c' command just continues execution to the end
            if ( tokenized_cmd[0] == "c") {
                break_instruction = -1;
                break_address = 0;
                step = 0;
            } else if (tokenized_cmd[0] == "g") {
                if ( tokenized_cmd.size() != 2 ) {
                    std::cout << "g <address>" << std::endl;
                    continue;
                }

                break_address = strtoull(tokenized_cmd[1].c_str(), NULL, 16);
                break_instruction = -1;
                step = 0;
            } else if (tokenized_cmd[0] == "t") {
                if ( tokenized_cmd.size() != 2 ) {
                    std::cout << "t <skip count>" << std::endl;
                    continue;
                }

                break_address = 0;
                break_instruction = ins_count + strtoull(tokenized_cmd[1].c_str(), NULL, 16);
                step = 0;
            } else if (tokenized_cmd[0] == "s") {
                break_address = 0;
                break_instruction = ins_count + 1;
                step = 1;
            } else if ( tokenized_cmd[0] == "set") {
                if ( tokenized_cmd.size() != 2 ) {
                    std::cout << "set [pr | pe | pi]" << std::endl;
                    continue;
                }

                if ( tokenized_cmd[1] == "pr") {
                    print_regs = 1;
                } else if ( tokenized_cmd[1] == "pe") {
                    print_ext = 1;
                } else if ( tokenized_cmd[1] == "pi") {
                    print_ins = 1;
                }

                continue;
            } else if ( tokenized_cmd[0] == "unset") {
                if ( tokenized_cmd.size() != 2 ) {
                    std::cout << "unset [pr | pe | pi]" << std::endl;
                    continue;
                }

                if ( tokenized_cmd[1] == "pr") {
                    print_regs = 0;
                } else if ( tokenized_cmd[1] == "pe") {
                    print_ext = 0;
                } else if ( tokenized_cmd[1] == "pi") {
                    print_ins = 0;
                }

                continue;
            } else if (tokenized_cmd[0] == "x") {
                if ( tokenized_cmd.size() != 2 ) {
                    std::cout << "x [<address> | <reg>]" << std::endl;
                    continue;
                }

                if ( cpu.isReg( tokenized_cmd[1]) ) {
                    read_addr = cpu.getreg( tokenized_cmd[1] );
                } else {
                    read_addr = strtoull(tokenized_cmd[1].c_str(), NULL, 16);
                }

                if (mem.isReadable( read_addr, 8) ) {
                    mem.read_l( (void*)read_addr, (char*)&read_value, 8);

                    std::cout << "0x" << read_addr << ":\t0x" << std::setfill('0') << std::setw(16) << read_value << std::endl;
                } else {
                    std::cout << "0x" << read_addr << ":\tCannot access memory" << std::endl;
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

        } catch ( SycallException e ) {
            if ( os.syscall_dispatch() ) {
                cpu.printState();
                std::cout << "[INFO] Total instructions: " << ins_count << std::endl;
                std::cout << "[INFO] Execution Completed" << std::endl;
                int waitforit;
                std::cin >> waitforit;
                std::cerr << "[INFO] Execution Completed" << std::endl;
                return 0;
            }

        } catch ( MemoryException e ) {
            std::cout << e.what() << std::endl;
            cpu.printState();
            return 0;
        } catch(const FloatingPointException &e) {            
            std::cout << e.what() << std::endl;
            //exit(0);
        } catch ( InstructionException e) {
            std::cout << e.what() << ": " << e.error_number << std::endl;
            return 0;
        }

        if ( print_regs ) {
            cpu.printState();
        }

        if ( print_ext ) {
            cpu.printExtendedState();
        }

        if ( ins_count >= break_instruction || break_address == cpu._rip.val) {
            step = 1;
        }
    }

    return 0;
}
