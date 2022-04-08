#include "range.h"
#include "heap.h"

#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

int link_byte_range( byte_range **root, byte_range *leaf)
{
	if ( root == NULL || leaf == NULL ) {
		return 1;
	}

	if ( *root == NULL ) {
		*root = leaf;

		return 0;
	}

	leaf->next = *root;
	*root = leaf;

	return 0;
}

byte_range *delete_invalid( byte_range *root, unsigned int filesize )
{
	byte_range *final = NULL;
	byte_range *walker = NULL;

	if ( root == NULL || filesize == 0) {
		return NULL;
	}

	while ( root ) {
		if ( root->start < 0 ) {
			if ( (root->start * -1) >= filesize ) {
				root->start = filesize * -1;
			}

			root->start = filesize - (root->start * -1);
			root->end = filesize;

			walker = root;

			root = root->next;
			walker->next = NULL;

			link_byte_range(&final, walker );

			continue;
		}

		if ( root->end >= filesize ) {
#ifndef PATCHED_2
#else
			root->end = filesize - 1;
#endif
		}

		if ( root->start >= filesize ) {
			walker = root->next;
			chfree(root);

			root = walker;

			continue;
		}

		walker = root;

		root = root->next;
		walker->next = NULL;

		link_byte_range(&final, walker );
	}

	return final;
}

byte_range *parse_single_range(char *line)
{
	byte_range *br = NULL;
	char *dash = NULL;

	if ( line == NULL ) {
		return NULL;
	}

	br = challoc( sizeof(byte_range) );

	if ( br == NULL ) {
		return NULL;
	}

	memset( br, 0, sizeof(byte_range) );

	dash = strchr( line, '-');

	if ( dash == NULL ) {
		chfree(br);

		return NULL;
	}
	
	if ( dash == line ) {
		br->start = atoi( line );

		if ( br->start > 0 ) {
			chfree(br);

			return NULL;
		}

		dash += 1;

		while( dash[0] ) {
			if ( !isdigit(dash[0]) ) {
				chfree(br);

				return NULL;
			}

			dash++;
		}

		return br;
	}

	dash[0] = 0x00;

	br->start = atoi(line);

	dash += 1;

	if ( strlen(dash) == 0 ) {
		br->end = -1;

		return br;
	}

	br->end = atoi(dash);	

	if ( br->start < 0 || br->end <= 0 ) {
		chfree(br);

		return NULL;
	}

	return br;
}

byte_range *parse_ranges( char *line )
{
	byte_range *range_list = NULL;
	byte_range *rwalker = NULL;

	char *walker = NULL;
	char *end = NULL;
	char *single = NULL;
	int l = 0;

	if ( line == NULL ) {
		goto cleanup;
	}

	if ( strncasecmp( line, "bytes=", 6) != 0 ) {
		goto cleanup;
	}

	walker = line + 6;

	while ( walker ) {
		end = strchr(walker, ',' );

		if ( end ) {
			l = end - walker;

			end = end + 1;			
		} else {
			l = strlen(walker);
		}

		single = challoc( l + 1);

		if ( single == NULL ) {
			goto cleanup;
		}

		memset( single, 0, l + 1);

		memcpy( single, walker, l);

		rwalker = parse_single_range( single );

		chfree(single);

		link_byte_range( &range_list, rwalker );

		walker = end;
	}

	return range_list;

cleanup:
	if ( single ) {
		chfree(single);
	}

	while ( range_list ) {
		rwalker = range_list->next;

		chfree(range_list);
		range_list = range_list->next;
	}

	return NULL;
}
