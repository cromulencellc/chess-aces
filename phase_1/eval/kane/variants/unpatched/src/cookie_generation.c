#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "base64.h"
#include "cookie_generation.h"
#define LENGTH_OF_BINARY 128
#define LENGTH_OF_ENCODING 32
char *generate_cookie(unsigned int seed) {
  unsigned int uuid[LENGTH_OF_BINARY + 1];
  char *encoded = calloc(1, LENGTH_OF_ENCODING + 1);
  srand(seed);
  int i;
  for (i = 0; i < LENGTH_OF_BINARY; ++i) {
    uuid[i] = rand() % 2;
  }
  uuid[48] = 0;
  uuid[49] = 1;
  uuid[50] = 0;
  uuid[51] = 0;
  uuid[64] = 1;
  uuid[65] = 0;
  i = 0;
  uuid[48] = 0;
  uuid[49] = 1;
  uuid[50] = 0;
  uuid[51] = 0;
  uuid[64] = 1;
  uuid[65] = 0;
  encoded = encode_to_hex(uuid, encoded);
  encoded = encode_base64(encoded);
  return encoded;
}
char *encode_to_hex(unsigned int *binary_stream, char *encoded_uuid) {
  int i, j, power = 3, power_total = 0;
  for (i = 0; i < LENGTH_OF_ENCODING; ++i) {
    for (j = 0; j < 4; ++j) {
      if (binary_stream[(i * 4) + j] == 1) {
        power_total += pow(2, power);
      }
      power--;
    }
    if (power_total == 10) {
      encoded_uuid[i] = 'a';
    } else if (power_total == 11) {
      encoded_uuid[i] = 'b';
    } else if (power_total == 12) {
      encoded_uuid[i] = 'c';
    } else if (power_total == 13) {
      encoded_uuid[i] = 'd';
    } else if (power_total == 14) {
      encoded_uuid[i] = 'e';
    } else if (power_total == 15) {
      encoded_uuid[i] = 'f';
    } else {
      encoded_uuid[i] = (char)power_total + 48;
    }
    power = 3;
    power_total = 0;
  }
  return encoded_uuid;
}
