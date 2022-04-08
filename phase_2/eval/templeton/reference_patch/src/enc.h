#ifndef __ENC_H__
#define __ENC_H__

enum encoding_type {
	gzip,
	compress,
	deflate,
	bzip2,
	identity,
	any,
	invalid_et,
};

typedef struct encoding {
	enum encoding_type et;
	double q;
	struct encoding *next;
} encoding;

encoding *parse_encodings( char * e);


#endif