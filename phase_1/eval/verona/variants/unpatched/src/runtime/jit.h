#ifndef jit_HEADER
#define jit_HEADER
#include <stdint.h>
#include "container/binary.h"
#define MEMORY_REGION_JIT_SIZE 0x00200000
#define MEMORY_REGION_CODE_SIZE 0x00010000
#define MEMORY_REGION_DATA_SIZE 0x00010000
struct memory_region {
  uint8_t jit[MEMORY_REGION_JIT_SIZE];
  uint8_t code[MEMORY_REGION_CODE_SIZE];
  uint8_t data[MEMORY_REGION_DATA_SIZE];
};
#define FLAG_BRANCH_SET 0x0001
#define FLAG_EQUAL 0x0002
#define FLAG_LESS_THAN_SIGNED 0x0004
#define FLAG_LESS_THAN_UNSIGNED 0x0008
#define FLAG_SYSCALL 0x0010
#define DEFAULT_STACK_POINTER 0xFFF0
struct context {
  uint16_t gprs[8];
  uint16_t pc;
  uint16_t flags;
  uint16_t branch_address;
  void *block_accelerator;
  struct memory_region *memory;
  struct aa_tree *jitted_instructions;
  uint32_t jit_offset;
} __attribute__((__packed__));
struct context *context_create();
void context_delete(struct context *context);
int context_reset_jit(struct context *context);
int context_load_binary(struct context *context, const struct binary *binary);
int context_step(struct context *context);
int jit_run(struct context *context);
#endif