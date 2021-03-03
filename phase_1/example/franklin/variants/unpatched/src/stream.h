#ifndef __STREAM__
#define __STREAM__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>

typedef struct stream {
    char *buffer;
    int index;

    
    int bit_index;
    int bits_remaining;
    int length;

    int fd;
    FILE *fp;
} stream;

stream *open_packet_file( char *fn );
stream *init_stream( uint8_t *data, uint32_t size);
int get_bits( stream *st, uint8_t *dest, uint32_t bits );
int get_single_byte( stream *st, uint8_t *byte );
int get_single_bit( stream *st, uint8_t *bit );
int return_bits( stream *st, uint32_t bits );
int bits_remaining( stream *st);
void set_fd( stream * st, int fd );
int write_data( stream *st, const char* format, ... );

#endif