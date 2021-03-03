#include "jit.h"
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "container/aa_tree.h"
#include "container/bb.h"
#include "error.h"
#include "instructions.h"
#include "jit_translate.h"
#include "rust.h"
struct jitted_instruction {
  uint16_t vm_address;
  struct instruction *instruction;
  uint8_t block_accelerator_set;
  uintptr_t jit_address;
};
struct jitted_instruction *
jitted_instruction_create(struct instruction *instruction,
                          uintptr_t jit_address) {
  struct jitted_instruction *ji =
      (struct jitted_instruction *)malloc(sizeof(struct jitted_instruction));
  ji->vm_address = instruction->address;
  ji->block_accelerator_set = 0;
  ji->instruction = instruction;
  ji->jit_address = jit_address;
  return ji;
}
uint16_t jitted_instruction_vm_address(const struct jitted_instruction *ji) {
  return ji->vm_address;
}
void jitted_instruction_delete(struct jitted_instruction *ji) {
  free(ji->instruction);
  free(ji);
}
int jitted_instruction_cmp(const struct jitted_instruction *lhs,
                           const struct jitted_instruction *rhs) {
  if (lhs->vm_address < rhs->vm_address) {
    return -1;
  } else if (lhs->vm_address > rhs->vm_address) {
    return 1;
  } else {
    return 0;
  }
}
struct context *context_create() {
  struct context *context = (struct context *)malloc(sizeof(struct context));
  memset(context, 0, sizeof(struct context));
  context->gprs[SP] = DEFAULT_STACK_POINTER;
  struct memory_region *memory;
  memory =
      mmap(0, sizeof(struct memory_region), PROT_READ | PROT_WRITE | PROT_EXEC,
           MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  context->memory = memory;
  context->jitted_instructions =
      aa_tree_create((aa_tree_cmp)&jitted_instruction_cmp);
  return context;
}
void context_delete(struct context *context) {
  munmap(context->memory, sizeof(struct memory_region));
  aa_tree_delete(context->jitted_instructions,
                 (aa_tree_del)&jitted_instruction_delete);
  free(context);
}
int context_reset_jit(struct context *context) {
  aa_tree_delete(context->jitted_instructions,
                 (aa_tree_del)&jitted_instruction_delete);
  context->jitted_instructions =
      aa_tree_create((aa_tree_cmp)&jitted_instruction_cmp);
  context->jit_offset = 0;
  context->block_accelerator = NULL;
  return 0;
}
int context_load_binary(struct context *context, const struct binary *binary) {
  memcpy(&context->memory->code, binary->code, binary->code_size);
  memcpy(&context->memory->data, binary->data, binary->data_size);
  context->pc = 0;
  context->gprs[SP] = DEFAULT_STACK_POINTER;
  return 0;
}
int context_step(struct context *context) {
  int (*j)(struct context *);
  if (context->block_accelerator != NULL) {
    j = (int (*)(struct context *))context->block_accelerator;
    context->block_accelerator = 0;
    return j(context);
  }
  struct jitted_instruction seek;
  seek.vm_address = context->pc;
  struct jitted_instruction *ji =
      aa_tree_fetch(context->jitted_instructions, &seek);
  if (ji == NULL) {
    struct instruction *instruction =
        (struct instruction *)malloc(sizeof(struct instruction));
    try_cleanup_start(instruction_decode(instruction, context->pc,
                                         &context->memory->code[context->pc],
                                         MEMORY_REGION_CODE_SIZE - context->pc))
        snprintf(error_description, ERROR_DESCRIPTION_SIZE,
                 "Could not decode instruction at 0x%04x", context->pc);
    try_cleanup_end struct bb *bb = bb_create(1024);
    if ((try_err = jit_translate(bb, instruction)) != SUCCESS) {
      bb_delete(bb);
      return try_err;
    }
    if (bb_length(bb) + context->jit_offset > MEMORY_REGION_JIT_SIZE) {
      if ((try_err = context_reset_jit(context)) != SUCCESS) {
        bb_delete(bb);
        return try_err;
      }
    }
    memcpy(&context->memory->jit[context->jit_offset], bb_data(bb),
           bb_length(bb));
    uintptr_t jit_address =
        (uintptr_t)&context->memory->jit[context->jit_offset];
    context->jit_offset += bb_length(bb);
    bb_delete(bb);
    ji = jitted_instruction_create(instruction, jit_address);
    aa_tree_insert(context->jitted_instructions, ji);
  }
  if ((ji->block_accelerator_set == 0) &&
      (ji->instruction->opcode != OP_CALL) &&
      (ji->instruction->opcode != OP_RET) &&
      (ji->instruction->opcode != OP_JMP) &&
      (ji->instruction->opcode != OP_JE) &&
      (ji->instruction->opcode != OP_JL) &&
      (ji->instruction->opcode != OP_JLE) &&
      (ji->instruction->opcode != OP_JB) &&
      (ji->instruction->opcode != OP_JBE) &&
      (ji->instruction->opcode != OP_SYSCALL)) {
    seek.vm_address = ji->vm_address + ji->instruction->length;
    struct jitted_instruction *jji =
        aa_tree_fetch(context->jitted_instructions, &seek);
    if (jji != NULL) {
      jit_set_block_accelerator((void *)ji->jit_address, jji->jit_address);
      ji->block_accelerator_set = 1;
    }
  }
  j = (int (*)(struct context *))ji->jit_address;
  context->block_accelerator = 0;
  return j(context);
}
void context_debug(const struct context *context) {
  struct instruction instruction;
  if (instruction_decode(&instruction, context->pc,
                         &context->memory->code[context->pc],
                         MEMORY_REGION_CODE_SIZE - context->pc) == 0) {
    char buf[256];
    instruction_sprintf(buf, 256, &instruction);
    printf("%04x: (", context->pc);
    unsigned int i;
    for (i = 0; i < instruction.length; i++) {
      printf("%02x", context->memory->code[context->pc + i]);
    }
    printf(") %s\n", buf);
  } else {
    printf("%04x: Invalid Instruction\n", context->pc);
  }
  printf(
      "r0:%04x r1:%04x r2:%04x  r3:%04x  r4:%04x  r5:%04x  r6:%04x r7:%04x\n",
      context->gprs[0], context->gprs[1], context->gprs[2], context->gprs[3],
      context->gprs[4], context->gprs[5], context->gprs[6], context->gprs[7]);
  printf("stack: %04x: %04x %04x %04x %04x\n", context->gprs[SP],
         *((uint16_t *)&context->memory->data[context->gprs[SP]]),
         *((uint16_t *)&context->memory->data[context->gprs[SP] + 2]),
         *((uint16_t *)&context->memory->data[context->gprs[SP] + 4]),
         *((uint16_t *)&context->memory->data[context->gprs[SP] + 6]));
  printf("flag-branch-set:%u flag-equal:%u flag-less-than-signed:%u\n",
         context->flags & FLAG_BRANCH_SET, context->flags & FLAG_EQUAL,
         context->flags & FLAG_LESS_THAN_SIGNED);
  printf("flag-less-than-unsigned:%u flag-syscall:%u\n",
         context->flags & FLAG_LESS_THAN_UNSIGNED,
         context->flags & FLAG_SYSCALL);
  printf("\n");
}
int jit_run(struct context *context) {
  unsigned int i;
  for (i = 0; i < 1024 * 1024; i++) {
    try
      (context_step(context)) if (context->flags & FLAG_SYSCALL) {
        switch (context->gprs[R0]) {
        case 0:
          return 0;
        case 1: {
          uint32_t size = context->gprs[R1];
          size += context->gprs[R2];
          if (size > MEMORY_REGION_DATA_SIZE) {
            context->gprs[R0] = -1;
            break;
          }
          int bytes_read = read(0, &(context->memory->data[context->gprs[R1]]),
                                context->gprs[R2]);
          context->gprs[R0] = bytes_read;
          break;
        }
        case 2: {
          uint32_t size = context->gprs[R1];
          size += context->gprs[R2];
          if (size > MEMORY_REGION_DATA_SIZE) {
            context->gprs[R0] = -1;
            break;
          }
          int bytes_written =
              write(1, &(context->memory->data[context->gprs[R1]]),
                    context->gprs[R2]);
          context->gprs[R0] = bytes_written;
          break;
        }
        default:
          context->gprs[R0] = -1;
          break;
        }
      }
    if (context->flags & FLAG_BRANCH_SET) {
      context->pc = context->branch_address;
    }
    context->flags &= (~FLAG_SYSCALL) & (~FLAG_BRANCH_SET);
  }
  return ERROR_TOO_MANY_INSTRUCTIONS;
}