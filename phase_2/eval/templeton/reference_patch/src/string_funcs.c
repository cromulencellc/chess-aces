#include "string_funcs.h"
#include "heap.h"

#include <string.h>
#include <stdio.h>

strbuf *init_string( char *s )
{
	if ( s == NULL ) {
		return NULL;
	}

	strbuf *nsb = challoc(sizeof(strbuf));

	if ( nsb == NULL ) {
		return NULL;
	}

	nsb->length = strlen( s );
	nsb->max = strlen( s ) * 2;
	nsb->index = 0;

	nsb->stream = challoc( nsb->max );

	if ( nsb->stream == NULL ) {
		chfree(nsb);
		return NULL;
	}

	memcpy( nsb->stream, s, nsb->length );

	return nsb;
}

strbuf *init_octets( char *s, int length )
{
	if ( s == NULL ) {
		return NULL;
	}

	strbuf *nsb = challoc(sizeof(strbuf));

	if ( nsb == NULL ) {
		return NULL;
	}

	nsb->length = length;
	nsb->max = length * 2;
	nsb->index = 0;

	nsb->stream = challoc( nsb->max );

	if ( nsb->stream == NULL ) {
		chfree(nsb);
		return NULL;
	}

	memcpy( nsb->stream, s, nsb->length );

	return nsb;
}

int append_string( strbuf *sb, char *s)
{
	if ( sb == NULL || s == NULL ) {
		return 0;
	}

	int append_length = strlen(s);

	if ( append_length == 0 ) {
		return sb->length;
	}

	if ( sb->max <= sb->length + append_length ) {
		char * tmp = challocre( sb->stream, sb->max + (sb->length + append_length) * 2 );

		if ( tmp == NULL ) {
			return 0;
		}

		sb->max = sb->max + (sb->length + append_length) * 2 ;

		sb->stream = tmp;
	}

	memcpy( sb->stream + sb->length, s, append_length );

	sb->length += append_length;

	return sb->length;
}

int append_octets( strbuf *sb, char *s, int length)
{
	if ( sb == NULL || s == NULL ) {
		return 0;
	}

	if ( length == 0 ) {
		return sb->length;
	}

	if ( sb->max <= sb->length + length ) {
		char * tmp = challocre( sb->stream, sb->max + (sb->length + length) * 2 );

		if ( tmp == NULL ) {
			return 0;
		}

		sb->max = sb->max + (sb->length + length) * 2 ;

		sb->stream = tmp;
	}

	memcpy( sb->stream + sb->length, s, length );

	sb->length += length;

	return sb->length;
}

int append_int( strbuf *sb, int i )
{
	char int_to_string[64];

	if ( sb == NULL ) {
		return 0;
	}

	snprintf(int_to_string, 64, "%d", i);

	return append_string( sb, int_to_string);
}

int append_strbuf( strbuf *sb, strbuf *end )
{
	if ( sb == NULL || end == NULL ) {
		return 0;
	}

	return append_octets( sb, end->stream, end->length);
}

int expand_string( strbuf *sb, size_t size )
{
	if ( sb == NULL ) {
		return 0;
	}

	char * temp = challocre( sb->stream, sb->max + size );

	if ( temp == NULL ) {
		return 0;
	}

	sb->stream = temp;
	sb->max = sb->max + size;

	return sb->max;
}

void free_string( strbuf *sb )
{
	if ( sb ) {
		if ( sb->stream ) {
			chfree(sb->stream);
		}

		chfree(sb);
	}

	return;
}

int get_length( strbuf *sb )
{
	if ( sb == NULL ) {
		return 0;
	}

	return sb->length;
}

char popchar( strbuf *sb )
{
	if ( sb == NULL ) {
		return 0;
	}

	if ( sb->length == 0 ) {
		return 0;
	}

	sb->length -= 1;

	return sb->stream[sb->length];
}