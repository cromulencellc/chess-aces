#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "ppm.h"
#define CHANNELS 3
#define WIDTH 250
#define LENGTH 87
#define WHITE 255
#define BLACK 0
PPM *create_ppm(char *filename, uint64_t version) {
  PPM *image = malloc(sizeof(PPM));
  if (image == NULL) {
    fprintf(stderr, "[CREATE_PPM] Failed to allocate memory for image\n");
    return NULL;
  }
  image->fp = print_ppm;
  strncpy(image->filename, filename, MAX_FILENAME - 1);
  image->filename[MAX_FILENAME - 1] = '\0';
  strncpy(image->magic_num, "P3", 2);
  image->magic_num[2] = '\0';
  image->width = WIDTH;
  image->length = LENGTH;
  image->version = version;
  FILE *fptr;
  if ((fptr = fopen(filename, "w")) == NULL) {
    fprintf(stderr, "[CREATE_PPM] Failed to open image file\n");
    free(image);
    image = NULL;
    return NULL;
  }
  fprintf(fptr, "P3\r\n");
  fprintf(fptr, "%d", WIDTH);
  fprintf(fptr, " ");
  fprintf(fptr, "%d\r\n", LENGTH);
  fprintf(fptr, "255\r\n");
  int i = 0, j = 0, k = 0;
  image->pixels = (int ***)malloc(WIDTH * sizeof(int **));
  if (image->pixels == NULL) {
    fprintf(stderr, "[CREATE_PPM] Failed to allocate first layer\n");
    free(image);
    image = NULL;
    return NULL;
  }
  for (i = 0; i < WIDTH; ++i) {
    image->pixels[i] = (int **)malloc(LENGTH * sizeof(int *));
    if (image->pixels[i] == NULL) {
      fprintf(stderr, "[CREATE_PPM] Failed to allocate second layer\n");
      free(image->pixels);
      image->pixels = NULL;
      free(image);
      image = NULL;
      return NULL;
    }
    for (j = 0; j < LENGTH; j++) {
      image->pixels[i][j] = (int *)malloc(CHANNELS * sizeof(int));
      if (image->pixels[i][j] == NULL) {
        fprintf(stderr, "[CREATE_PPM] Failed to allocate third layer\n");
        free(image->pixels[i]);
        image->pixels[i] = NULL;
        free(image->pixels);
        image->pixels = NULL;
        free(image);
        image = NULL;
      }
      for (k = 0; k < CHANNELS; k++) {
        image->pixels[i][j][k] = WHITE;
        fprintf(fptr, " %d ", image->pixels[i][j][k]);
      }
      fprintf(fptr, " ");
    }
    fprintf(fptr, "\n");
  }
  fclose(fptr);
  return image;
}
void print_ppm(uint16_t width, uint16_t length, int ***pixels, uint64_t dummy) {
}
void destroy_ppm(PPM *p) {
  if (p == NULL) {
    fprintf(stderr, "[DESTROY_PPM] The ppm image was already freed\n");
    return;
  }
  if (p->pixels == NULL) {
    fprintf(stderr,
            "[DESTROY_PPM] The ppm image does not have allocated pixels\n");
    return;
  }
  int i = 0, j = 0;
  for (i = 0; i < p->width; ++i) {
    for (j = 0; j < p->length; ++j) {
      free(p->pixels[i][j]);
      p->pixels[i][j] = NULL;
    }
    free(p->pixels[i]);
    p->pixels[i] = NULL;
  }
  free(p->pixels);
  free(p);
}
