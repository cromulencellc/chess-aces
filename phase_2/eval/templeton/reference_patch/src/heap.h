#ifndef __HEAP_H__
#define __HEAP_H__

#include <stdint.h>
#include <sys/mman.h>

typedef struct block {
	uint32_t cookie;

	// The MSB of prev_size is the hole bit
	// It indicates if there is a block following it
	uint16_t prev_size;

	// The MSB of curr_size is the free bit so 
	// 	there are only 15 bits available.
	// Set to 0 if free, 1 if allocated
	uint16_t curr_size;
	struct block *next;
	struct block *prev;
} block;

#define DATATOBLOCK( ptr ) ( (block *)((uint8_t*)ptr - sizeof(block)))
#define SETFREE( ptr ) (ptr->curr_size &= 0x7fff)
#define SETALLOCD( ptr ) ptr->next = NULL; ptr->prev = NULL; ptr->curr_size |= 0x8000
#define SETHOLE( ptr ) ( ptr->prev_size |= 0x8000 )
#define CLEARHOLE( ptr ) ( ptr->prev_size &= 0x7fff )
#define ISFREE( ptr ) ( (ptr->curr_size & 0x8000) == 0 )
#define ISHOLE( ptr ) ( (ptr->prev_size & 0x8000) != 0 )
#define DATAPTR( ptr ) ( (uint8_t*)ptr + sizeof(block) )
#define PREVSZ( ptr ) (ptr->prev_size & 0x7fff)
#define BLOCKSZ( ptr ) (ptr->curr_size & 0x7fff)

#define MAXSIZE		0x7fff

typedef struct heap_meta {
	uint32_t free_bytes;
	uint32_t allocd_bytes;
	uint32_t free_count;

	// Each bucket is 8 bytes
	// Bucket 0 is for blocks larger than 1016 bytes
	block *blocks[128];
} heap_meta;

void *challoc( uint32_t size );
void *challocre( void *ptr, uint32_t size);
void chfree( void *ptr );

/* Heap management functions */
int init_heap( void );
void printit();

#endif
