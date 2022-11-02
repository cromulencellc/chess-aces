/* Copyright (c) 2007-2013 Sam Trenholme
 *
 * TERMS
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * This software is provided 'as is' with no guarantees of correctness or
 * fitness for purpose.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* A header that can be included by DwHash.c */
#define HEADER "/* This file is automatically generated by RandomPrime */\n" \
"\n" \
"#define MUL_CONSTANT %d \n"

/* Find a random 31-bit prime number */

/* Is the number we're looking at a 31-bit prime number?  0 if not, 1
 * if it is */
int isprime(uint32_t candidate) {
        uint32_t a = 0;
        /* Make sure it's a 31-bit odd number */
        if((candidate & 0xc0000001) != 0x40000001) {
                return 0;
        }

        /* Quick and dirty test; this doesn't scale up very well.  In
         * particular, it isn't fast enough to be practical for finding
         * 64-bit primes */
        for(a = 3; a < 46341; a++) {
                if((candidate % a) == 0) {
                        return 0;
                }
        }
        return 1;
}

/* How many 1 bits does this number have? */
int num_1bits(uint32_t candidate) {
        int a = 0;
        int out = 0;
        for(a=0;a<32;a++) {
                if((candidate & 0x01) == 0x01) {
                        out++;
                }
                candidate >>= 1;
        }

        return out;
}

/* Die on fatal error */
void fatal(char *why) {
        printf("Fatal: %s\n",why);
        exit(1);
}

/* Get 32 bits of entropy and convert the entropy in to a 31-bit-long
 * prime number */
int main() {
        FILE *rand = 0;
        uint32_t candidate = 0;
        uint8_t get = 0;
        int a = 0;

        rand = fopen("/dev/urandom","rb");
        if(rand == 0) {
                fatal("Could not open /dev/urandom");
        }

        for(a = 0; a < 4; a++) {
                get = getc(rand);
                candidate <<= 8;
                candidate |= get;
        }

        fclose(rand);

        candidate &= 0x3fffffff;
        candidate |= 0x40000001;

        while(isprime(candidate) == 0 || num_1bits(candidate) < 16) {
                candidate += 2;
                candidate &= 0x3fffffff;
                candidate |= 0x40000001;
        }

        printf(HEADER,candidate);
        printf("/* %d has %d bits set to 1 */\n",candidate,
                num_1bits(candidate));

        return 0;
}

