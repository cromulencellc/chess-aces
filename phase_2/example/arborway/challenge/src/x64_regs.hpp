#ifndef __X64_REGS_HPP__
#define __X64_REGS_HPP__

#include "register.hpp"

rtype x64_regs[] = { { "al",    "rax",  LOWBYTE,    8},
                     { "ah",    "rax",  HIGHBYTE,   8},
                     { "ax",    "rax",  WORD,       16},
                     { "eax",   "rax",  DWORD,      32},
                     { "rax",   "rax",  QWORD,      64},

                     { "bl",    "rbx",  LOWBYTE,    8},
                     { "bh",    "rbx",  HIGHBYTE,   8},
                     { "bx",    "rbx",  WORD,       16},
                     { "ebx",   "rbx",  DWORD,      32},
                     { "rbx",   "rbx",  QWORD,      64},

                     { "cl",    "rcx",  LOWBYTE,    8},
                     { "ch",    "rcx",  HIGHBYTE,   8},
                     { "cx",    "rcx",  WORD,       16},
                     { "ecx",   "rcx",  DWORD,      32},
                     { "rcx",   "rcx",  QWORD,      64},

                     { "dl",    "rdx",  LOWBYTE,    8},
                     { "dh",    "rdx",  HIGHBYTE,   8},
                     { "dx",    "rdx",  WORD,       16},
                     { "edx",   "rdx",  DWORD,      32},
                     { "rdx",   "rdx",  QWORD,      64},

                     { "sil",   "rsi",  LOWBYTE,    8},
                     { "si",    "rsi",  WORD,       16},
                     { "esi",   "rsi",  DWORD,      32},
                     { "rsi",   "rsi",  QWORD,      64},

                     { "dil",   "rdi",  LOWBYTE,    8},
                     { "di",    "rdi",  WORD,       16},
                     { "edi",   "rdi",  DWORD,      32},
                     { "rdi",   "rdi",  QWORD,      64},

                     { "spl",   "rsp",  LOWBYTE,    8},
                     { "sp",    "rsp",  WORD,       16},
                     { "esp",   "rsp",  DWORD,      32},
                     { "rsp",   "rsp",  QWORD,      64},

                     { "bpl",   "rbp",  LOWBYTE,    8},
                     { "bp",    "rbp",  WORD,       16},
                     { "ebp",   "rbp",  DWORD,      32},
                     { "rbp",   "rbp",  QWORD,      64},

                     { "r8b",   "r8",   LOWBYTE,    8},
                     { "r8w",   "r8",   WORD,       16},
                     { "r8d",   "r8",   DWORD,      32},
                     { "r8",    "r8",   QWORD,      64},

                     { "r9b",   "r9",   LOWBYTE,    8},
                     { "r9w",   "r9",   WORD,       16},
                     { "r9d",   "r9",   DWORD,      32},
                     { "r9",    "r9",   QWORD,      64},

                     { "r10b",  "r10",  LOWBYTE,    8},
                     { "r10w",  "r10",  WORD,       16},
                     { "r10d",  "r10",  DWORD,      32},
                     { "r10",   "r10",  QWORD,      64},

                     { "r11b",  "r11",  LOWBYTE,    8},
                     { "r11w",  "r11",  WORD,       16},
                     { "r11d",  "r11",  DWORD,      32},
                     { "r11",   "r11",  QWORD,      64},

                     { "r12b",  "r12",  LOWBYTE,    8},
                     { "r12w",  "r12",  WORD,       16},
                     { "r12d",  "r12",  DWORD,      32},
                     { "r12",   "r12",  QWORD,      64},

                     { "r13b",  "r13",  LOWBYTE,    8},
                     { "r13w",  "r13",  WORD,       16},
                     { "r13d",  "r13",  DWORD,      32},
                     { "r13",   "r13",  QWORD,      64},

                     { "r14b",  "r14",  LOWBYTE,    8},
                     { "r14w",  "r14",  WORD,       16},
                     { "r14d",  "r14",  DWORD,      32},
                     { "r14",   "r14",  QWORD,      64},

                     { "r15b",  "r15",  LOWBYTE,    8},
                     { "r15w",  "r15",  WORD,       16},
                     { "r15d",  "r15",  DWORD,      32},
                     { "r15",   "r15",  QWORD,      64},

                     { "ip",    "rip",  WORD,       16},
                     { "eip",   "rip",  DWORD,      32},
                     { "rip",   "rip",  QWORD,      64},
                            
                     { "xmm0",  "ymm0", DDWORD,     128},
                     { "ymm0",  "ymm0", DQWORD,     256},

                     { "xmm1",  "ymm1", DDWORD,     128},
                     { "ymm1",  "ymm1", DQWORD,     256},

                     { "xmm2",  "ymm2", DDWORD,     128},
                     { "ymm2",  "ymm2", DQWORD,     256},

                     { "xmm3",  "ymm3", DDWORD,     128},
                     { "ymm3",  "ymm3", DQWORD,     256},

                     { "xmm4",  "ymm4", DDWORD,     128},
                     { "ymm4",  "ymm4", DQWORD,     256},

                     { "xmm5",  "ymm5", DDWORD,     128},
                     { "ymm5",  "ymm5", DQWORD,     256},

                     { "xmm6",  "ymm6", DDWORD,     128},
                     { "ymm6",  "ymm6", DQWORD,     256},

                     { "xmm7",  "ymm7", DDWORD,     128},
                     { "ymm7",  "ymm7", DQWORD,     256},

                     { "xmm8",  "ymm8", DDWORD,     128},
                     { "ymm8",  "ymm8", DQWORD,     256},

                     { "xmm9",  "ymm9", DDWORD,     128},
                     { "ymm9",  "ymm9", DQWORD,     256},

                     { "xmm10",  "ymm10", DDWORD,     128},
                     { "ymm10",  "ymm10", DQWORD,     256},

                     { "xmm11",  "ymm11", DDWORD,     128},
                     { "ymm11",  "ymm11", DQWORD,     256},

                     { "xmm12",  "ymm12", DDWORD,     128},
                     { "ymm12",  "ymm12", DQWORD,     256},

                     { "xmm13",  "ymm13", DDWORD,     128},
                     { "ymm13",  "ymm13", DQWORD,     256},

                     { "xmm14",  "ymm14", DDWORD,     128},
                     { "ymm14",  "ymm14", DQWORD,     256},

                     { "xmm15",  "ymm15", DDWORD,     128},
                     { "ymm15",  "ymm15", DQWORD,     256},

                     { "xmm16",  "ymm16", DDWORD,     128},
                     { "ymm16",  "ymm16", DQWORD,     256},

                     { "xmm17",  "ymm17", DDWORD,     128},
                     { "ymm17",  "ymm17", DQWORD,     256},

                     { "xmm18",  "ymm18", DDWORD,     128},
                     { "ymm18",  "ymm18", DQWORD,     256},

                     { "xmm19",  "ymm19", DDWORD,     128},
                     { "ymm19",  "ymm19", DQWORD,     256},

                     { "xmm20",  "ymm20", DDWORD,     128},
                     { "ymm20",  "ymm20", DQWORD,     256},

                     { "xmm21",  "ymm21", DDWORD,     128},
                     { "ymm21",  "ymm21", DQWORD,     256},

                     { "xmm22",  "ymm22", DDWORD,     128},
                     { "ymm22",  "ymm22", DQWORD,     256},

                     { "xmm23",  "ymm23", DDWORD,     128},
                     { "ymm23",  "ymm23", DQWORD,     256},

                     { "xmm24",  "ymm24", DDWORD,     128},
                     { "ymm24",  "ymm24", DQWORD,     256},

                     { "xmm25",  "ymm25", DDWORD,     128},
                     { "ymm25",  "ymm25", DQWORD,     256},

                     { "xmm26",  "ymm26", DDWORD,     128},
                     { "ymm26",  "ymm26", DQWORD,     256},

                     { "xmm27",  "ymm27", DDWORD,     128},
                     { "ymm27",  "ymm27", DQWORD,     256},

                     { "xmm28",  "ymm28", DDWORD,     128},
                     { "ymm28",  "ymm28", DQWORD,     256},

                     { "xmm29",  "ymm29", DDWORD,     128},
                     { "ymm29",  "ymm29", DQWORD,     256},

                     { "xmm30",  "ymm30", DDWORD,     128},
                     { "ymm30",  "ymm30", DQWORD,     256},

                     { "xmm31",  "ymm31", DDWORD,     128},
                     { "ymm31",  "ymm31", DQWORD,     256}

                            };





#endif