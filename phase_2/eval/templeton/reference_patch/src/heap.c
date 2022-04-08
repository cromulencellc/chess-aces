#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "heap.h"

block *coalesce( block *b );

uint32_t heap_cookie;
heap_meta m;

int gen_heap_cookie( )
{
	int fd = open("/dev/urandom", O_RDONLY);

	if ( fd <= 0 ) {
		return 1;
	}

	if ( read(fd, &heap_cookie, sizeof(heap_cookie)) <= 0 ) {
		return 1;
	}
	
	close(fd);

	return 0;
}

int init_heap( void )
{
	if ( gen_heap_cookie( ) ) {
		return 1;
	}

	block *p = mmap( NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	if ( p == NULL ) {
		return 1;
	}

	p->cookie = heap_cookie;
	p->prev_size = 0;
	p->curr_size = 4096 - sizeof(block);
	p->next = NULL;
	p->prev = NULL;


	m.free_bytes = 4096 - sizeof(block);
	m.allocd_bytes = 0;

	memset(m.blocks, 0, 128 * sizeof(block*));

	m.blocks[0] = p;
	m.free_count = 0;

	return 0;
}

block *create_block( uint32_t size )
{
	block *nb = NULL;

	size += sizeof(block);

	if ( size % 4096 ) {
		size = (size + 4096) & 0xfffff000;
	}

	if ( size > MAXSIZE) {
		return NULL;
	}

	nb = mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);

	nb->cookie = heap_cookie;
	nb->prev_size = 0;
	nb->curr_size = size - sizeof(block);
	nb->next = NULL;
	nb->prev = NULL;

	m.free_bytes += (size - sizeof(block));

	return nb;
}

int unlink_block( block *b)
{
	int index;

	if ( BLOCKSZ(b) <= 1016 ) {
		index = BLOCKSZ(b) / 8;

		if ( b->prev == NULL ) {
			if ( m.blocks[index] != b ) {
				exit(1);
			}

			m.blocks[index] = b->next;
			m.free_bytes -= BLOCKSZ(b);

			if ( m.blocks[index] ) {
				m.blocks[index]->prev = NULL;
			}

			b->next = NULL;
			b->prev = NULL;

			return 0;
		} else if ( b->next == NULL ) {
			if ( b->prev->next != b ) {
				exit(1);
			}

			b->prev->next = NULL;
			m.free_bytes -= BLOCKSZ(b);

			b->next = NULL;
			b->prev = NULL;

			return 0;
		} else {
			b->prev->next = b->next;
			b->next->prev = b->prev;

			m.free_bytes -= BLOCKSZ(b);

			b->next = NULL;
			b->prev = NULL;
			return 0;
		}
	} else {
		if ( b->prev == NULL ) {
			if ( m.blocks[0] != b ) {
				exit(1);
			}

			m.blocks[0] = b->next;

			if ( m.blocks[0] ) {
				m.blocks[0]->prev = NULL;
			}

			m.free_bytes -= BLOCKSZ(b);

			b->next = NULL;
			b->prev = NULL;

			return 0;
		} else if ( b->next == NULL ) {
			if ( b->prev->next != b ) {
				exit(1);
			}

			b->prev->next = NULL;
			m.free_bytes -= BLOCKSZ(b);

			b->next = NULL;
			b->prev = NULL;

			return 0;
		} else {
			b->prev->next = b->next;
			b->next->prev = b->prev;

			m.free_bytes -= BLOCKSZ(b);

			b->next = NULL;
			b->prev = NULL;
			return 0;
		}
	}

	return 0;
}

void insert_block( block *b)
{
	block *walker = NULL;

	if ( b == NULL ) {
		return;
	}

	SETFREE(b);

	b = coalesce(b);

	b->next = NULL;
	b->prev = NULL;

	if ( BLOCKSZ(b) <= 1016 ) {
		int index = BLOCKSZ(b) / 8;
		b->next = m.blocks[index];
		m.blocks[index] = b;

		if ( b->next ) {
			b->next->prev = b;
		}

	} else {
		if (m.blocks[0] == NULL ) {
			m.blocks[0] = b;
			m.blocks[0]->prev = NULL;

		} else if (BLOCKSZ(m.blocks[0]) < BLOCKSZ(b) ) {
			b->next = m.blocks[0];
			m.blocks[0]->prev = b;
			m.blocks[0] = b;
			m.blocks[0]->prev = NULL;
		} else {
			walker = m.blocks[0];

			while ( walker ) {
				if (walker->cookie != heap_cookie) {
					fprintf(stderr, "heap cookie check fail!\n");
					exit(1);
				}

				if ( walker->next == NULL ) {
					walker->next = b;
					b->prev = walker;

					break;
				} else if ( BLOCKSZ(walker->next) <= BLOCKSZ(b) ) {
					b->next = walker->next;
					b->prev = walker;

					b->next->prev = b;
					walker->next = b;
					break;
				} else {
					walker = walker->next;
				}
			}
		}
	}

	b->cookie = heap_cookie;

	m.free_bytes += BLOCKSZ(b);

	return;
}

block *coalesce( block *b )
{
	if ( b == NULL ) {
		return b;
	}

	if ( PREVSZ(b) == 0 ) {
		return b;
	}

	uint8_t *tb = (uint8_t *)b;
	tb -= PREVSZ(b);
	tb -= sizeof(block);

	block *prev_block = (block*)tb;

	if ( prev_block->cookie != heap_cookie ) {
		fprintf(stderr, "bad heap cookie\n");

		exit(1);
	}

	if ( !ISFREE(prev_block) ) {
		return b;
	}

	if ( unlink_block(prev_block) ) {
		exit(0);
	}

	prev_block->curr_size = BLOCKSZ(prev_block) + sizeof(block) + BLOCKSZ(b);

	if ( ISHOLE(b) ) {
		block *f = (block*)(DATAPTR(b) + BLOCKSZ(b));

		int i = ISHOLE(f);

		f->prev_size = BLOCKSZ(prev_block);

		if ( i ) {
			SETHOLE(f);
		}

	} else {
		CLEARHOLE(prev_block);
	}

	memset(b, 0, sizeof(block));

	return prev_block;
}

int collect_garbage( )
{
	block *walker = m.blocks[0];
	block *temp = NULL;

	while ( walker ) {
		if ( walker->cookie != heap_cookie ) {
			fprintf(stderr, "[ERROR] heap cookie check fail\n");

			exit(1);
		}

		if ( PREVSZ(walker) != 0 ) {
			temp = (block *)( ((uint8_t*)walker) - (PREVSZ(walker) + sizeof(block)));

			if ( temp->cookie != heap_cookie ) {
				exit(1);
			}

			if ( ISFREE(temp) )  {
				unlink_block(walker);

				insert_block( walker );

				walker = m.blocks[0];

				continue;
			}

		}

		if ( ISHOLE(walker) ) {
			temp = (block *)(DATAPTR(walker) + BLOCKSZ(walker));

			if ( temp->cookie != heap_cookie ) {
				fprintf(stderr, "bad cookie\n");

				exit(1);
			}

			if ( ISFREE(temp) )  {
				unlink_block(temp);

				insert_block( temp );

				walker = m.blocks[0];

				continue;
			}
		}

		walker = walker->next;
	}

	int reset = 0;

	for ( int x = 1; x < 128; x++) {
		walker = m.blocks[x];

		if ( reset ) {
			reset = 0;
		}

		while ( walker ) {
			if ( walker->cookie != heap_cookie ) {
				fprintf(stderr, "[ERROR] heap cookie check fail\n");

				exit(1);
			}

			if ( PREVSZ(walker) != 0 ) {
				temp = (block *)( ((uint8_t*)walker) - (PREVSZ(walker) + sizeof(block)));

				if ( temp->cookie != heap_cookie ) {
					fprintf(stderr, "bad cookie\n");

					exit(1);
				}

				if ( ISFREE(temp) )  {
					unlink_block(walker);
					insert_block( walker );

					x = 1;

					reset = 1;
					break;
				}

			}

			if ( ISHOLE(walker) ) {

				temp = (block *)(DATAPTR(walker) + BLOCKSZ(walker));

				if ( temp->cookie != heap_cookie ) {
					fprintf(stderr, "bad cookie\n");

					exit(1);
				}

				if ( ISFREE(temp) )  {
					unlink_block(temp);
					insert_block( temp );

					x = 1;
					reset = 1;
					break;
				}
			}

			walker = walker->next;
		}
	}

	return 0;
}

int add_large_block( uint32_t size )
{
	block *nb = create_block( size );

	if ( nb == NULL ) {
		return 1;
	}

	insert_block( nb );

	return 0;
}

block *get_large( uint32_t size )
{
	if ( size % 8 ) {
		size = (size + 8) & 0xfffffff8;
	}

	if ( m.blocks[0] == NULL ) {
		return NULL;
	} else if (BLOCKSZ(m.blocks[0]) < size) {
		return NULL;
	}

	block *walker = m.blocks[0];

	while ( walker ) {

		if (walker->cookie != heap_cookie) {
			fprintf(stderr, "heap cookie check fail!\n");

			exit(1);
		
		}
		if ( walker->next == NULL ) {
			if ( walker->prev == NULL ) {
				m.blocks[0] = NULL;

			} else {
				walker->prev->next = NULL;
				walker->prev = NULL;
			}

			m.free_bytes -= BLOCKSZ(walker);

			return walker;
		} else {
			if ( BLOCKSZ(walker->next) <= size ) {

				if ( walker->prev == NULL ) {
					m.blocks[0] = walker->next;
					m.blocks[0]->prev = NULL;

					walker->next = NULL;
				} else {
					walker->prev->next = walker->next;
					walker->next->prev = walker->prev;

					walker->prev = NULL;
					walker->next = NULL;
				}

				m.free_bytes -= BLOCKSZ(walker);
				return walker;
			} else {
				walker = walker->next;
			}
		}
	}

	return NULL;
}

void *alloc_large( uint32_t size )
{
	if ( size % 8 ) {
		size = (size + 8) & 0xfffffff0;
	}

	if ( m.blocks[0] == NULL) {
		if ( add_large_block( size ) ) {
			return NULL;
		}
	} else if ( BLOCKSZ(m.blocks[0]) < size ) {
		if ( add_large_block( size ) ) {
			return NULL;
		}
	}

	block *b = get_large( size );

	if ( b == NULL ) {
		return NULL;
	}
	
	b->next = NULL;
	b->prev = NULL;

	m.allocd_bytes += BLOCKSZ(b);

	if ( (BLOCKSZ(b) - size) < 32 ) {
		SETALLOCD( b );

		return DATAPTR(b);
	} else {
		block *new_block = (block*)(DATAPTR(b) + size);

		new_block->curr_size = BLOCKSZ(b) - (size + sizeof(block)); 
		new_block->prev_size = size;

		if ( ISHOLE( b ) ) {
			SETHOLE( new_block );

			block *f = (block*)(DATAPTR(new_block) + BLOCKSZ(new_block));

			int h = ISHOLE(f);

			f->prev_size = BLOCKSZ(new_block);

			if ( h ) {
				SETHOLE(f);
			}
		} else {
			SETHOLE( b );
		}

		b->curr_size = size;

		SETALLOCD(b);

		insert_block(new_block);
		
		return DATAPTR(b);
	}

	return NULL;
}

void *challoc( uint32_t size )
{
	if ( size < 8 ) {
		size = 8;
	}

	if ( size > MAXSIZE ) {
		return NULL;
	}

	if ( size % 8 ) {
		size = (size + 8) & 0xfffffff8;
	}

	if ( size > 1016 ) {
		void *a = alloc_large(size);

		if ( a == NULL ) {
			return a;
		}

		block *z = DATATOBLOCK(a);
		memset(a, 0, BLOCKSZ(z));

		return a;
	} else {
		block *b = NULL;

		int index = size / 8;

		while ( index < 128 ) {
			if (m.blocks[index] != NULL ) {
				b = m.blocks[index];

				unlink_block(b);

				break;
			} else {
				index++;
			}
		}

		if ( b == NULL ) {
			b = get_large( size );

			if ( b == NULL ) {
				add_large_block(size);

				b = get_large(size);
			}

			if ( b == NULL ) {
				return NULL;
			}
		}

		if ( (BLOCKSZ(b) - size) < 32 ) {
			SETALLOCD( b );

		} else {
			block *new_block = (block*)(DATAPTR(b) + size);

			new_block->curr_size = BLOCKSZ(b) - (size + sizeof(block)); 
			new_block->prev_size = size;

			if ( ISHOLE( b ) ) {
				SETHOLE( new_block );

				block *f = (block*)(DATAPTR(new_block) + BLOCKSZ(new_block));

				int h = ISHOLE(f);

				f->prev_size = BLOCKSZ(new_block);

				if ( h ) {
					SETHOLE(f);
				}
			} else {
				SETHOLE( b );
			}

			b->curr_size = size;
			SETALLOCD(b);

			insert_block(new_block);
		}

		void *a = DATAPTR(b);

		memset(a, 0, BLOCKSZ(b));

		return a;
	}

	return NULL;
}

void chfree( void *ptr )
{
	if ( ptr == NULL ) {
		return;
	}

	block *b = DATATOBLOCK( ptr );

	if ( b->cookie != heap_cookie ) {
		fprintf(stderr, "[ERROR] heap overflow detected\n");

		exit(1);
	}

	if ( ISFREE(b) ) {
		fprintf(stderr, "[ERROR] double free detected\n");

		exit(1);
	}

	insert_block( b );

	m.free_count += 1;

	if ( (m.free_count % 25) == 0 ) {
		collect_garbage();
	}

	return;
}

void *challocre( void *ptr, uint32_t size)
{
	block *old_block = NULL;
	block *new_block = NULL;
	int copy_length = 0;

	if ( ptr == NULL ) {
		return ptr;
	}

	old_block = DATATOBLOCK(ptr);

	new_block = challoc( size );

	if ( new_block == NULL ) {
		return NULL;
	}

	copy_length = ( size > BLOCKSZ(old_block) ) ? BLOCKSZ(old_block) : size;

	memcpy( new_block, ptr, copy_length);

	chfree(ptr);

	return new_block;
}
