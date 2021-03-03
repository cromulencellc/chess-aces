#include "binary.h"
#include <stdlib.h>
#include <string.h>
#include "error.h"
struct binary *binary_create(const uint8_t *code, uint32_t code_size,
                             const uint8_t *data, uint32_t data_size) {
  struct binary *binary = malloc(sizeof(struct binary));
  binary->code = malloc(code_size);
  binary->data = malloc(data_size);
  memcpy(binary->code, code, code_size);
  memcpy(binary->data, data, data_size);
  binary->code_size = code_size;
  binary->data_size = data_size;
  return binary;
}
int binary_parse(struct binary **binary, const uint8_t *data, uint32_t len) {
  if (len < 4) {
    return ERROR_BINARY_TOO_SMALL;
  }
  uint16_t code_size = data[0];
  code_size <<= 8;
  code_size |= data[1];
  if (code_size + 4 > len) {
    return ERROR_BINARY_CODE_TOO_LARGE;
  }
  uint16_t data_size = data[code_size + 2];
  data_size <<= 8;
  data_size |= data[code_size + 3];
  if (code_size + data_size + 4 > len) {
    return ERROR_BINARY_DATA_TOO_LARGE;
  }
  *binary = (struct binary *)malloc(sizeof(struct binary));
  (*binary)->code_size = code_size;
  (*binary)->data_size = data_size;
  (*binary)->code = malloc(code_size);
  (*binary)->data = malloc(data_size);
  memcpy((*binary)->code, &(data[2]), code_size);
  memcpy((*binary)->data, &(data[4 + code_size]), data_size);
  return SUCCESS;
}
void binary_delete(struct binary *binary) {
  free(binary->code);
  free(binary->data);
  free(binary);
}