#ifndef PPM_H
#define PPM_H
#include <stdint.h>
#define MAX_FILENAME 255
typedef void (*print)(uint16_t, uint16_t, int ***, uint64_t);
typedef struct PPM {
  print fp;
  char filename[MAX_FILENAME];
  char magic_num[3];
  uint16_t width;
  uint16_t length;
  uint64_t version;
  int ***pixels;
} PPM;
PPM *create_ppm(char *filename, uint64_t version);
void print_ppm(uint16_t width, uint16_t length, int ***, uint64_t version);
void destroy_ppm(PPM *p);
#endif
