#ifndef __RANGE_H__
#define __RANGE_H__

typedef struct byte_range {
	int start;
	int end;

	struct byte_range *next;	
} byte_range;

byte_range *parse_ranges( char *line );
byte_range *delete_invalid( byte_range *root, unsigned int filesize );

#endif