#ifndef binary_HEADER
#define binary_HEADER
#include <stdint.h>
struct binary {
  uint16_t code_size;
  uint8_t *code;
  uint16_t data_size;
  uint8_t *data;
};
struct binary *binary_create(const uint8_t *code, uint32_t code_size,
                             const uint8_t *data, uint32_t data_size);
int binary_parse(struct binary **binary, const uint8_t *data, uint32_t len);
void binary_delete(struct binary *binary);
#endif