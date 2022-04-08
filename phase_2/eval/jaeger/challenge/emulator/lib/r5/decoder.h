#ifndef decoder_HEADER
#define decoder_HEADER

#include <stdint.h>

#include "instruction.h"

int r5_decode(struct r5_instruction * ins, uint32_t word);

#endif