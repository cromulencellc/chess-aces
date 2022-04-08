#ifndef PGM_H
#define PGM_H

#include <stdint.h>

#define MAX_FILENAME 255

typedef void (*printer_func)(uint16_t, uint16_t, int **, uint64_t);

typedef struct PGM {

  char filename[MAX_FILENAME];

  printer_func fp;

  char magic_num[3];

  uint16_t width;

  uint16_t length;

  uint64_t version;

  int **pixels;

} PGM;

PGM *create_pgm(char *filename, uint64_t version);
void print_pgm(uint16_t width, uint16_t length, int **pixels, uint64_t version);
int is_pgm(char *bytes);
void destroy_pgm(PGM *p);

#endif
