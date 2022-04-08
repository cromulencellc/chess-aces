#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pgm.h"
#define WHITE 255
#define BLACK 0
#define WIDTH 262
#define LENGTH 250
PGM *create_pgm(char *filename, uint64_t version) {
  if (filename == NULL) {
    fprintf(stderr, "[CREATE_PGM] There was no filename given\n");
    return NULL;
  }
  PGM *image = malloc(sizeof(PGM));
  if (image == NULL) {
    fprintf(stderr, "[CREATE_PGM] Failed to allocate memory for image\n");
    return NULL;
  }
  strncpy(image->filename, filename, MAX_FILENAME - 1);
  image->filename[MAX_FILENAME - 1] = '\0';
  image->fp = print_pgm;
  strncpy(image->magic_num, "P2", 2);
  image->magic_num[2] = '\0';
  image->width = WIDTH;
  image->length = LENGTH;
  image->version = version;
  FILE *fptr;
  if ((fptr = fopen(filename, "w")) == NULL) {
    fprintf(stderr, "[CREATE_PGM] Failed to open image file\n");
    free(image);
    image = NULL;
    return NULL;
  }
  fprintf(fptr, "P2\r\n");
  fprintf(fptr, "%d", WIDTH);
  fprintf(fptr, " ");
  fprintf(fptr, "%d\r\n", LENGTH);
  fprintf(fptr, "255\r\n");
  int i = 0, j = 0;
  image->pixels = (int **)malloc(WIDTH * sizeof(int *));
  if (image->pixels == NULL) {
    fprintf(stderr, "[CREATE_PGM] Failed to allocate first layer\n");
    free(image);
    image = NULL;
    return NULL;
  }
  for (i = 0; i < WIDTH; ++i) {
    image->pixels[i] = (int *)malloc(LENGTH * sizeof(int));
    if (image->pixels == NULL) {
      fprintf(stderr, "[CREATE_PGM] Failed to allocate second layer\n");
      free(image->pixels);
      image->pixels = NULL;
      free(image);
      image = NULL;
      return NULL;
    }
    for (j = 0; j < LENGTH; ++j) {
      image->pixels[i][j] = WHITE;
      fprintf(fptr, " %d ", image->pixels[i][j]);
    }
    fprintf(fptr, "\n");
  }
  fclose(fptr);
  return image;
}
void print_pgm(uint16_t width, uint16_t length, int **pixels,
               uint64_t version) {}
void destroy_pgm(PGM *p) {
  if (p == NULL) {
    fprintf(stderr, "[DESTROY_PGM] The PGM passed was already freed\n");
    return;
  }
  if (p->pixels == NULL) {
    fprintf(stderr, "[DESTROY_PGM] There are no pixels to free\n");
    return;
  } else {
    int i = 0;
    for (i = 0; i < p->width; ++i) {
      free(p->pixels[i]);
      p->pixels[i] = NULL;
    }
    free(p->pixels);
    p->pixels = NULL;
  }
  free(p);
}
