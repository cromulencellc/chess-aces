#ifndef __STREAM__
#define __STREAM__

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/// This stream structure operates at byte level 
typedef struct stream {
    char *data;
    size_t length;
    size_t index;
} stream;

stream *initstream( char *data, size_t length );
stream *init_stream_nd( size_t length );
int readbyte( stream *s, char *c );
char *readlength( stream *s, size_t length );
char *readuntil( stream *s, char c);
char *eatitall( stream *s);
int incstream( stream *s );
int skip_white_space( stream *s);
int add_bytes( stream *s, char *nd, size_t length);
int add_string( stream *s, char *nd);
int add_int( stream *s, int v);
void free_stream(stream *s);
#endif