#ifndef buf_build_HEADER
#define buf_build_HEADER
#include <stdint.h>
struct bb {
  uint32_t allocated_size;
  uint32_t current_length;
  uint8_t buf[];
};
struct bb *bb_create(uint32_t allocated_size);
void bb_delete(struct bb *bb);
int bb_push(struct bb *bb, uint8_t byte);
int bb_append(struct bb *bb, const void *data, uint32_t length);
const uint8_t *bb_data(const struct bb *bb);
uint32_t bb_length(const struct bb *bb);
#endif