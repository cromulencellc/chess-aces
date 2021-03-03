#ifndef jit_translate_HEADER
#define jit_translate_HEADER
#include "container/bb.h"
#include "instructions.h"
int jit_translate(struct bb *bb, struct instruction *instruction);
int jit_set_block_accelerator(uint8_t *code, uint64_t address);
#endif