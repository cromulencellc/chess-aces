#include "bb.h"
#include <stdlib.h>
#include <string.h>
struct bb *bb_create(uint32_t allocated_size) {
  struct bb *bb = (struct bb *)malloc(sizeof(struct bb) + allocated_size);
  if (bb == NULL) {
    return NULL;
  }
  bb->allocated_size = allocated_size;
  bb->current_length = 0;
  return bb;
}
void bb_delete(struct bb *bb) { free(bb); }
int bb_push(struct bb *bb, uint8_t byte) {
  if (bb->current_length == bb->allocated_size) {
    return -1;
  }
  bb->buf[bb->current_length++] = byte;
  return 0;
}
int bb_append(struct bb *bb, const void *data, uint32_t length) {
  if (bb->current_length + length > bb->allocated_size) {
    return -1;
  }
  memcpy(&(bb->buf[bb->current_length]), data, length);
  bb->current_length += length;
  return 0;
}
const uint8_t *bb_data(const struct bb *bb) { return bb->buf; }
uint32_t bb_length(const struct bb *bb) { return bb->current_length; }