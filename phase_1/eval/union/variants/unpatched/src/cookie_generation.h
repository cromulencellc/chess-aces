#ifndef COOKIE_GENERATION_H
#define COOKIE_GENERATION_H
#include "header.h"
char *generate_cookie(unsigned int seed);
char *encode_to_hex(unsigned int *binary_stream, char *encoded_uuid);
#endif
