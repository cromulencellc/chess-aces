#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"
static char encoding_table[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
char *encode_base64(char *string_to_encode) {
  char tri_window[3][9];
  char intermed_binary[25];
  char six_bit_values[4][3];
  char *encoded =
      calloc(1, ((((strlen(string_to_encode) - 1) / 3) + 1) * 4) + 5);
  char *eight_bit_binary = malloc(9);
  int six_bit_total = 0;
  char *convert_copy = strdup(string_to_encode);
  int i = 0, j = 0;
  for (i = 0; i < strlen(convert_copy); i += 3) {
    for (j = 0; j < 3; ++j) {
      eight_bit_binary =
          convert_to_binary(convert_copy[i + j], eight_bit_binary, 8);
      strncpy(tri_window[j], eight_bit_binary, 8);
      tri_window[j][8] = '\0';
    }
    strncpy(intermed_binary, tri_window[0], 9);
    strncat(intermed_binary, tri_window[1], 9);
    strncat(intermed_binary, tri_window[2], 9);
    int k = 0, power = 0;
    for (k = 0; k < 4; ++k) {
      for (j = ((k + 1) * 6) - 1; j >= k * 6; --j) {
        if (intermed_binary[j] == '1') {
          six_bit_total += pow(2, power);
        }
        power++;
      }
      snprintf(six_bit_values[k], 3, "%d", six_bit_total);
      six_bit_total = 0;
      power = 0;
    }
    if (i == 0) {
      snprintf(encoded, 5, "%c%c%c%c", encoding_table[atoi(six_bit_values[0])],
               encoding_table[atoi(six_bit_values[1])],
               encoding_table[atoi(six_bit_values[2])],
               encoding_table[atoi(six_bit_values[3])]);
      encoded[5] = '\0';
    } else {
      char *new_encoding = calloc(1, 5);
      snprintf(new_encoding, 5, "%c%c%c%c",
               encoding_table[atoi(six_bit_values[0])],
               encoding_table[atoi(six_bit_values[1])],
               encoding_table[atoi(six_bit_values[2])],
               encoding_table[atoi(six_bit_values[3])]);
      strcat(encoded, new_encoding);
      free(new_encoding);
      new_encoding = NULL;
    }
    if (strlen(convert_copy) % 3 == 2 && i + 3 > strlen(convert_copy)) {
      encoded[strlen(encoded) - 1] = '=';
    }
    if (strlen(convert_copy) % 3 == 1 && i + 3 > strlen(convert_copy)) {
      encoded[strlen(encoded) - 1] = '=';
      encoded[strlen(encoded) - 2] = '=';
    }
  }
  strncpy(string_to_encode, encoded, 32);
  free(convert_copy);
  convert_copy = NULL;
  free(eight_bit_binary);
  eight_bit_binary = NULL;
  return encoded;
}
char *decode_base64(char *string_to_decode) {
  if (strlen(string_to_decode) % 4 != 0) {
    fprintf(stderr,
            "The strlen() of the %s you are attempting to decode is: %ld\n",
            string_to_decode, strlen(string_to_decode));
    fprintf(stderr,
            "The string you are trying to decode is not properly encoded\n");
    return NULL;
  }
  char *decoded = calloc(1, 1369);
  char *six_hold = calloc(1, 1024);
  int v = 0, equals_count = 0;
  for (v = 0; v < strlen(string_to_decode); ++v) {
    if (string_to_decode[v] == '=') {
      equals_count++;
    }
  }
  int i = 0, z = 0, y = 0;
  for (z = 0; z < strlen(string_to_decode); z += 4) {
    int j = 0;
    char b64_values[100][100];
    memset(b64_values, 0, 100 * 100);
    for (i = z, y = 0; i < z + 4; y++, i++) {
      for (j = 0; j < sizeof(encoding_table); ++j) {
        if (encoding_table[j] == string_to_decode[i]) {
          snprintf(b64_values[y], 3, "%d", j);
        }
        if (j == strlen(encoding_table - 1)) {
          strncpy(b64_values[y], "0", 1);
          b64_values[y][1] = '\0';
        }
      }
    }
    char six_bit_values[100][100];
    memset(six_bit_values, 0, 100 * 100);
    int k = 0;
    for (k = 0; k < 4; ++k) {
      six_hold = convert_to_binary(atoi(b64_values[k]), six_hold, 6);
      strncpy(six_bit_values[k], six_hold, 6);
      six_bit_values[k][6] = '\0';
    }
    char intermed_binary[2800];
    memset(intermed_binary, 0, 2800);
    strncpy(intermed_binary, six_bit_values[0], 7);
    strncat(intermed_binary, six_bit_values[1], 7);
    strncat(intermed_binary, six_bit_values[2], 7);
    strncat(intermed_binary, six_bit_values[3], 7);
    char eight_bit_values[500][500];
    memset(eight_bit_values, 0, 500 * 500);
    int p = 0, q = 0;
    for (p = 0; p < 3; ++p) {
      for (q = 0; q < 8; ++q) {
        eight_bit_values[p][q] = intermed_binary[(p * 8) + q];
      }
    }
    int m = 0, n = 0, power = 0, eight_bit_total = 0;
    char ascii_vals[50][50];
    memset(ascii_vals, 0, 50 * 50);
    for (m = 0; m < 3; ++m) {
      for (n = 7; n >= 0; --n) {
        if (eight_bit_values[m][n] == '1') {
          eight_bit_total += pow(2, power);
        }
        power++;
      }
      snprintf(ascii_vals[m], 4, "%d", eight_bit_total);
      eight_bit_total = 0;
      power = 0;
    }
    if (z == 0) {
      snprintf(decoded, sizeof(int) * 3, "%c%c%c", atoi(ascii_vals[0]),
               atoi(ascii_vals[1]), atoi(ascii_vals[2]));
    } else {
      char *new_decoding = calloc(1, 1024);
      snprintf(new_decoding,
               strlen(ascii_vals[0]) + strlen(ascii_vals[1]) +
                   strlen(ascii_vals[2]),
               "%c%c%c", atoi(ascii_vals[0]), atoi(ascii_vals[1]),
               atoi(ascii_vals[2]));
      strncat(decoded, new_decoding, 1024);
      free(new_decoding);
      new_decoding = NULL;
    }
  }
  free(six_hold);
  six_hold = NULL;
  return decoded;
}
char *convert_to_binary(char a, char *binary, int binary_len) {
  int i = 0, n = a;
  for (i = binary_len - 1; i >= 0; --i) {
    binary[i] = 48 + (n % 2);
    n = n / 2;
  }
  return binary;
}
