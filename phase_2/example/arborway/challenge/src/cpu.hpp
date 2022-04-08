#ifndef __CPU_HPP__
#define __CPU_HPP__

#include <stdint.h>
#include "memory.hpp"
#include "utils.hpp"
#include "register.hpp"

#include <iostream>
#include <map>

#include <capstone/capstone.h>
#include <capstone/platform.h>

int sign_extend( uint64_t in, uint64_t *out, uint64_t size);

class Cpu {
    public:
        Memory *mem;

        csh handle;
        cs_insn *insn;

        void *fs_segment;
        void *gs_segment;

        reg _rax;
        reg _rcx;
        reg _rdx;
        reg _rbx;
        reg _rsp;
        reg _rbp;
        reg _rdi;
        reg _rsi;
        reg _rip;
        reg _r8;
        reg _r9;
        reg _r10;
        reg _r11;
        reg _r12;
        reg _r13;
        reg _r14;
        reg _r15;
        
        reg _ymm0;
        reg _ymm1;
        reg _ymm2;
        reg _ymm3;
        reg _ymm4;
        reg _ymm5;
        reg _ymm6;
        reg _ymm7;
        reg _ymm8;
        reg _ymm9;
        reg _ymm10;
        reg _ymm11;
        reg _ymm12;
        reg _ymm13;
        reg _ymm14;
        reg _ymm15;
        reg _ymm16;
        reg _ymm17;
        reg _ymm18;
        reg _ymm19;
        reg _ymm20;
        reg _ymm21;
        reg _ymm22;
        reg _ymm23;
        reg _ymm24;
        reg _ymm25;
        reg _ymm26;
        reg _ymm27;
        reg _ymm28;
        reg _ymm29;
        reg _ymm30;
        reg _ymm31;

        reg _rflags;
        reg _cs;
        reg _ss;
        reg _ds;
        reg _es;
        reg _fs;
        reg _gs;

        /// This maps a register name to an actual class
        std::map< std::string, reg *> mapToRegClass;

        /// Map a portion of a register to its registry structure
        std::map< std::string, rtype *> mapToRegStruct;

        uint8_t cf; // carry flag
        uint8_t pf; // parity flag
        uint8_t af; // adjust flag
        uint8_t zf; // zero flag
        uint8_t sf; // sign flag
        uint8_t tf; // trap flag
        uint8_t iff; //interrupt enable flag
        uint8_t df; // direction flag
        uint8_t of; // overflow flag
        uint8_t iopl; // io privilege level
        uint8_t nt; // nested task flag;

        uint64_t rdtsc_value;

        std::map<std::string, std::string> regToReg64;

        Cpu( Memory *mem);
        Cpu( );
        ~Cpu( );

        void setupRegMap( );
        std::string mapToReg64( std::string reg );
        void setupRegClassMap( void );

        bool isReg( std::string reg );

        void init_fs_segment( void );

        int can_exec( uint64_t addr );

        void printState( );
        void printStateGDB( );
        void printExtendedState( );

        uint64_t setreg( std::string reg, uint64_t value);
        uint256_t setreg( std::string reg, uint256_t value);

        uint64_t getreg( std::string reg );
        uint256_t getymm( std::string reg );
        uint64_t step( uint64_t print_flag );

        int read_segment_mem( std::string reg, uint64_t offset, char *value, uint64_t length );
        int write_segment_mem( std::string reg, uint64_t offset, char *data, uint64_t length);

        int read_mem_operand( cs_x86_op *readop, uint64_t *value );
        int read_mem_operand( cs_x86_op *readop, uint256_t *value );

        int write_mem_operand( cs_x86_op *writeop, uint64_t value );
        int write_mem_operand( cs_x86_op *writeop, uint256_t value );

        /// Instruction handlers
        int add( cs_insn * insn);   // 8
        int and_ins( cs_insn * insn);   // 25
        int bsf( cs_insn * insn);   // 48
        int bsr( cs_insn * insn);   // 49
        int bt( cs_insn * insn);   // 56
        int call( cs_insn * insn);   // 56
        int cdqe( cs_insn * insn);   // 59
        int clc( cs_insn *insn);    // 62
        int cmova( cs_insn * insn);   // 71
        int cmovb( cs_insn * insn);   // 73
        int cmovbe( cs_insn * insn);   // 74
        int cmove( cs_insn * insn);   // 77
        int cmovne( cs_insn * insn);   // 85
        int cmovs( cs_insn * insn);   // 94
        int cmp( cs_insn * insn);   // 95
        int cmpxchg( cs_insn * insn);   // 100
        int cpuid( cs_insn * insn);   // 109
        int cqo( cs_insn * insn);   // 110
        int dec( cs_insn * insn);   // 133
        int div_ins( cs_insn * insn);   // 134
        int ret( cs_insn * insn);   // 147
        int movaps( cs_insn * insn);   // 198
        int idiv( cs_insn * insn);   // 211
        int imul( cs_insn * insn);   // 213
        int imul_single( cs_insn * insn);   // 213
        int imul_double( cs_insn * insn);   // 213
        int imul_triple( cs_insn * insn);   // 213

        int inc( cs_insn *insn); // 215
        int jae( cs_insn * insn);   // 253
        int ja( cs_insn * insn);   // 254
        int jbe( cs_insn * insn);   // 255
        int jb( cs_insn * insn);   // 256
        int je( cs_insn * insn);   // 259
        int jge( cs_insn * insn);   // 260
        int jg( cs_insn * insn);   // 261
        int jle( cs_insn * insn);   // 262
        int jl( cs_insn * insn);   // 263
        int jmp( cs_insn * insn);   // 264
        int jne( cs_insn * insn);   // 264
        int js( cs_insn * insn);   // 272
        int lea( cs_insn * insn);   // 322
        int leave( cs_insn * insn);   // 323
        int or_ins( cs_insn * insn);   // 332
        int sub( cs_insn * insn);   // 333
        int xor_ins( cs_insn * insn);   // 334
        int movd(cs_insn *insn); // 367
        int movq(cs_insn *insn); // 371
        int paddd(cs_insn *insn); // 379
        int paddq(cs_insn *insn); // 380
        int pcmpeqb( cs_insn * insn);   // 391
        int pcmpeqd( cs_insn * insn);   // 392
        int pcmpgtd( cs_insn * insn); // 395
        int pmovmskb( cs_insn * insn);   // 411
        int pslld( cs_insn *insn); // 424
        int psllq( cs_insn *insn); // 425
        int psubb( cs_insn * insn);   // 432
        int punpckhdq(cs_insn *insn); // 441
        int punpckldq(cs_insn *insn); // 444
        int pxor(cs_insn * insn); // 446
        int mov( cs_insn * insn);   // 449
        int movdqa(cs_insn *insn); // 453
        int movdqu(cs_insn *insn); // 454
        int movss(cs_insn *insn); // 475
        int movsx(cs_insn *insn); // 477
        int movsxd( cs_insn * insn);   // 478
        int movups( cs_insn * insn);   // 480
        int movzx( cs_insn * insn);   // 481
        int mul( cs_insn * insn);   // 483
        int neg( cs_insn * insn);   // 493
        int nop( cs_insn * insn);   // 494
        int not_ins( cs_insn * insn);   // 495
        int pop( cs_insn * insn);   // 566
        int pslldq( cs_insn * insn);   // 582
        int punpckhqdq(cs_insn *insn); // 586
        int punpcklqdq(cs_insn *insn); // 587
        int push( cs_insn * insn);   // 588
        int rdtsc( cs_insn * insn);   // 604
        int rol( cs_insn * insn);   // 606
        int ror( cs_insn * insn);   // 607
        int sar( cs_insn * insn);   // 619
        int sbb( cs_insn * insn);   // 621
        int seta( cs_insn * insn);   // 627
        int setb( cs_insn * insn);   // 629
        int sete( cs_insn * insn);   // 630
        int setg( cs_insn * insn);   // 632
        int setne( cs_insn * insn);   // 635
        int shl( cs_insn * insn);   // 651
        int shr( cs_insn * insn);   // 654
        int stc( cs_insn *insn);  // 670
        int stosq( cs_insn *insn);  // 677
        int syscall( cs_insn * insn);   // 695
        int test( cs_insn * insn);   // 700
        int tzcnt(cs_insn * insn);  // 703
        int vmovd(cs_insn *insn); // 924
        int vmovdqa(cs_insn *insn); // 927
        int vmovdqu(cs_insn *insn); // 932
        int vpbroadcastb(cs_insn *insn); // 997
        int vpcmpeqb( cs_insn *insn);   // 1007
        int vpminub( cs_insn *insn);   // 1121
        int vpmovmskb(cs_insn *insn); //1131
        int vpor(cs_insn *insn); //1167
        int vpxor( cs_insn *insn); // 1233
        int vzeroupper(cs_insn *insn); // 1292
        int xgetbv( cs_insn * insn);   // 1308
};


#endif