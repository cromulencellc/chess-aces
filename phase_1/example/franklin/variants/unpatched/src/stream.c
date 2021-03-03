#include "stream.h"

stream *init_stream( uint8_t *data, uint32_t size)
{
    stream *st = NULL;

    if ( !data ) {
        return NULL;
    }

    st = malloc( sizeof(stream) );

    if ( !st ) {
        return NULL;
    }

    memset( st, 0, sizeof(stream));

    st->buffer = malloc( size );

    if ( !st->buffer ) {
        free(st);
        return NULL;
    }

    memcpy( st->buffer, data, size );

    st->length = size;
    st->bits_remaining = size * 8;

    return st;
}

int write_data( stream *st, const char* format, ... ) {
    int bytes_written = 0;
    va_list args;
    
    if ( !st ) {
        return 0;
    }

    va_start( args, format );

    bytes_written = vfprintf(st->fp, format, args );

    va_end( args );

    return bytes_written;
}

void set_fd( stream *st, int fd )
{
    if ( !st ) {
        return;
    }

    st->fd = fd;

    st->fp = fdopen(fd, "r+");

    if ( !st->fp ) {
        fprintf(stderr, "[ERROR] Failed to open fd\n");
        exit(1);
    }
    return;
}

int bits_remaining( stream *st)
{
    if ( st == NULL ) {
        return 0;
    }

    return st->bits_remaining;
}

int return_bits( stream *st, uint32_t bits )
{
    if ( !st ) {
        return 0;
    }

    
    

    if ( bits + st->bits_remaining > st->length * 8 ) {
        return 0;
    }

    for ( int i = 0; i < bits; i++ ) {
        if ( st->bit_index ) {
            st->bit_index -= 1;
        } else {
            st->bit_index = 7;

            st->index -= 1;
        }

        st->bits_remaining += 1;
    }

    return bits;
}

int get_single_bit( stream *st, uint8_t *bit )
{
    uint8_t current_byte;

    if ( !st || !bit ) {
        return 0;
    }

    if ( !st->bits_remaining ) {
        return 0;
    }

    current_byte = st->buffer[ st->index ];

    current_byte >>= (7 - st->bit_index);

    current_byte &= 0x1;

    st->bit_index += 1;

    if ( st->bit_index % 8 == 0 ) {
        st->bit_index = 0;
        st->index += 1;
    }

    st->bits_remaining -= 1;

    *bit = current_byte;

    return 1;
}

int get_single_byte( stream *st, uint8_t *byte )
{
    uint8_t bit;
    uint8_t current_byte;

    if ( !st || !byte ) {
        return 0;
    }

    if ( st->bits_remaining < 8 ) {
        return 0;
    }

    for ( int i = 0; i < 8; i++ ) {
        get_single_bit( st, &bit);

        current_byte = ( current_byte << 1 ) | (bit & 0x1 );
    }

    *byte = current_byte;

    return 8;
}




int get_bits( stream *st, uint8_t *dest, uint32_t bits )
{
    uint32_t bytes_to_read = 0;
    uint32_t bits_to_read = 0;
    uint8_t final_bits = 0;
    uint8_t sbit = 0;

    if ( st == NULL || dest == NULL ) {
        return 0;
    }

    if ( st->bits_remaining < bits ) {
        return 0;
    }

    bytes_to_read = bits / 8;

    if ( bits % 8 ) {
        bits_to_read = bits % 8;
    }

    for ( int i = 0; i < bytes_to_read; i++ ) {
        get_single_byte( st, (uint8_t *)dest + i );
    }

    for ( int i = 0; i < bits_to_read; i++) {
        get_single_bit(st, &sbit);

        final_bits = (final_bits<<1) | (sbit & 0x1);
    }

    if ( bits_to_read ) {
        dest[bytes_to_read] = final_bits;
    }

    return bits;
}


stream *open_packet_file( char *fn )
{
    struct stat st;
    char *data;
    int fd;
    stream *file_stream;

    if ( fn == NULL ) {
        return NULL;
    }

    memset( &st, 0, sizeof(st));

    if ( stat(fn, &st) ) {
        fprintf(stderr, "[ERROR] Failed to open file: %s\n", fn);
        return NULL;
    }

    if ( st.st_size > 1024 ) {
        fprintf(stderr, "[ERROR] Packet file size too big\n");
        return NULL;
    }

    data = malloc ( st.st_size );

    if ( !data ) {
        fprintf(stderr, "[ERROR] Failed to allocate data block\n");
        return NULL;
    }

    fd = open( fn, O_RDONLY );

    if ( fd <= 0 ) {
        fprintf(stderr, "[ERROR] Failed to open %s for reading\n", fn);
        free( data );
        return NULL;
    }

    read( fd, data, st.st_size);

    close(fd);

    file_stream = malloc( sizeof(stream) );

    if ( !file_stream ) {
        fprintf(stderr, "[ERROR] Failed to allocate data\n");
        free( data );
        return NULL;
    }

    memset( file_stream, 0, sizeof(stream) );

    file_stream->buffer = data;
    file_stream->length = st.st_size;
    file_stream->bits_remaining = st.st_size * 8;

    return file_stream;
}