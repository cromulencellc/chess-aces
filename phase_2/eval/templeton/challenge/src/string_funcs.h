#ifndef __STRING_FUNCS_H__
#define __STRING_FUNCS_H__

#include <stddef.h>

typedef struct strbuf {
	char *stream;
	int max;
	int length;
	int index;
} strbuf;

int append_string( strbuf *sb, char *s);
int append_int( strbuf *sb, int i );
int append_octets( strbuf *sb, char *s, int length);
int append_strbuf( strbuf *sb, strbuf *end );
strbuf *init_string( char *s );
strbuf *init_octets( char *s, int length );
int expand_string( strbuf *sb, size_t size);
void free_string( strbuf *sb );
int get_length( strbuf *sb );
char popchar( strbuf *sb );


#endif