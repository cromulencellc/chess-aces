#include "stream.h"

stream *initstream( char *data, size_t length )
{
    stream *ns = NULL;

    if ( !data ) {
        goto end;
    }

    ns = calloc( 1, sizeof(stream) );

    if ( !ns ) {
        goto end;
    }

    ns->data = calloc( 1, length + 1 );

    if ( !ns->data ) {
        free(ns);
        ns = NULL;
        goto end;
    }

    memcpy( ns->data, data, length );
    ns->length = length;

end:
    return ns;
}

stream *init_stream_nd( size_t length )
{
    stream *ns = NULL;

    ns = calloc( 1, sizeof(stream) );

    if ( !ns ) {
        goto end;
    }

    ns->data = calloc( 1, length );

    if ( !ns->data ) {
        free(ns);
        ns = NULL;
        goto end;
    }

    ns->length = length;

end:
    return ns; 
}

int readbyte( stream *s, char *c )
{
    int result = 1;

    if (!s | !c) {
        goto end;
    }

    if ( s->index == s->length ) {
        goto end;
    }

    *c = s->data[ s->index++ ];
    result = 0;

end:
    return result;
}

char *readlength( stream *s, size_t length )
{
    char *d = NULL;

    if ( !s ) {
        goto end;
    }

    if ( s->index + length < s->length ) {
        goto end;
    }

    d = calloc( 1, length + 1 );

    if ( !d ) {
        goto end;
    }

    memcpy( d, s->data + s->index, length );
    s->index += length;

end:
    return d;
}

char *eatitall( stream *s)
{
    char *all = NULL;

    if ( !s ) {
        goto end;
    }

    if ( s->index == s->length ) {
        goto end;
    }

    all = calloc( 1, (s->length - s->index) + 1 );

    if ( !all ) {
        goto end;
    }

    memcpy( all, s->data + s->index, s->length - s->index );

    s->index = s->length;

end:
    return all;
}

/// Returns the buffer up to the specified character.
/// If the character isn't fount then it returns null.
/// The stream will point to the character upon success
char *readuntil( stream *s, char c)
{
    char *end = NULL;
    char *data = NULL;
    char *search_start = NULL;
    size_t length = 0;

    if ( !s ) {
        goto end;
    }

    search_start = s->data + s->index;

    end = strchr( search_start, c);

    if ( end == NULL ) {
        goto end;
    }

    length = end - search_start;

    data = calloc( 1, length + 1);

    if ( !data ) {
        goto end;
    }

    memcpy( data, search_start, length);
    s->index += length;

end: 
    return data;

}

int incstream( stream *s )
{
    int result = 1;

    if ( !s ) {
        goto end;
    }

    if ( s->index == s->length ) {
        goto end;
    }

    s->index++;

    result = 0;

end: 
    return result;
}

int skip_white_space( stream *s)
{
    int result = 1;

    if ( s == NULL ) {
        goto end;
    }

    while ( s->index < s->length && isspace(s->data[ s->index ]) ) {
        s->index++;
    }

    result = 0;

end:
    return result;

}

int add_bytes( stream *s, char *nd, size_t length)
{
    int result = 0;
    char *temp_buffer = NULL;
    size_t new_length = 0;

    if ( !s || !nd) {
        goto end;
    }

    if ( (s->index + length) >= s->length ) {
        
        new_length = (s->length + length) * 2;

        if ( new_length < (s->length + length) * 2 )  {
            printf("[ERROR] Integer overflow\n");
            abort();
        }

        temp_buffer = calloc( 1, new_length );

        if ( !temp_buffer ) {
            abort();
        }

        memset( temp_buffer, 0, new_length);

        memcpy( temp_buffer, s->data, s->index);

        free( s->data );

        s->data = temp_buffer;

        s->length = new_length;
    }

    memcpy( s->data + s->index, nd, length );

    s->index += length;

    result = s->index;
end:
    return result;
}

int add_string( stream *s, char *nd)
{
    int result = 0;

    if ( !s || !nd ) {
        goto end;
    }

    result = add_bytes( s, nd, strlen(nd) );

end:
    return result;
}

int add_int( stream *s, int v)
{
    int result = 0;
    char data[48];

    if ( !s ) {
        goto end;
    }

    memset( data, 0, 48);
    snprintf( data, 32, "%d", v);

    result = add_string( s, data );

end:
    return result;
}

void free_stream(stream *s)
{
    if ( !s ) {
        return;
    }

    if ( s->data ) {
        free(s->data);
    }

    free(s);

    return;
}
