#include "enc.h"
#include "heap.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

enum encoding_type get_enc_type( char *type )
{
	if ( type == NULL ) {
		return invalid_et;
	}

	if ( strcmp( type, "gzip") == 0 ) {
		return gzip;
	} else if ( strcmp( type, "compress") == 0 ) {
		return compress;
	} else if ( strcmp( type, "deflate") == 0 ) {
		return deflate;
	} else if ( strcmp( type, "bzip2") == 0 ) {
		return bzip2;
	} else if ( strcmp( type, "identity") == 0 ) {
		return identity;
	} else if ( strcmp( type, "*") == 0 ) {
		return any;
	}

	return invalid_et;
}

double get_q_value( char *q )
{
	char *eq = NULL;
	double v = 0.0;

	if ( q == NULL ) {
		return 0.0;
	}

	if ( strcmp(q, "q=") == 0 ) {
		return 0.0;
	}

	eq = strchr(q, '=');

	if ( eq == NULL ) {
		return 0.0;
	}
	
	v = strtod( eq + 1, NULL );

	if ( v > 1.0 ) {
		v = 1.0;
	}

	if ( v < 0.0 ) {
		v = 0.0;
	}

	return v;
}

encoding *parse_single_encoding( char *e )
{
	char *semi = NULL;
	char *type = NULL;
	char *q = NULL;
	int l = 0;

	encoding *enc = NULL;

	if ( e == NULL ) {
		return NULL;
	}

	enc = challoc( sizeof( encoding ) );

	if ( enc == NULL ) {
		return NULL;
	}

	memset(enc, 0, sizeof(encoding));

	semi = strchr( e, ';');

	if ( semi == NULL ) {
		enc->et = get_enc_type( e );

		if ( enc->et == invalid_et ) {
			chfree(enc);

			return NULL;
		}

		enc->q = 1.0;

		return enc;
	}

	l = semi - e;

	type = challoc( l + 1 );

	if ( type == NULL ) {
		chfree(enc);

		return NULL;
	}

	memset( type, 0, l + 1);
	memcpy( type, e, l);

	enc->et = get_enc_type( type );

	chfree(type);

	if ( enc->et == invalid_et ) {
		chfree(enc);

		return NULL;
	}

	l = strlen( semi + 1 );
	q = challoc( l + 1);

	if ( q == NULL ) {
		chfree( enc );

		return NULL;
	}

	memset( q, 0, l + 1);
	memcpy( q, semi + 1, l);

	enc->q = get_q_value( q );

	return enc;

}

int encoding_there( encoding **root, encoding *leaf )
{
	if ( *root == NULL || leaf == NULL ) {
		return 0;
	}

	encoding *walker = *root;

	while ( walker ) {
		if ( walker->et == leaf->et ) {
			return 1;
		}

		walker = walker->next;
	}

	return 0;
}

void append_encoding( encoding **root, encoding *leaf )
{
	encoding *walker = NULL;

	if ( root == NULL || leaf == NULL ) {
		return;
	}

	if ( encoding_there( root, leaf) ) {
		chfree( leaf );

		return;
	}

	if ( *root == NULL ) {
		*root = leaf;

		return;
	}

	if ( (*root)->q < leaf->q ) {
		leaf->next = *root;
		*root = leaf;

		return;
	}

	walker = *root;

	while( walker->next ) {
		if ( (*root)->next->q < leaf->q ) {
			leaf->next = (*root)->next;
			(*root)->next = leaf;

			return;
		}

		walker = walker->next;
	}

	walker->next = leaf;
	leaf->next = NULL;

	return;
}

encoding *parse_encodings( char * e)
{
	encoding * root = NULL;
	encoding * accepted_encoding = NULL;
	encoding * walker = NULL;

	char *single = NULL;
	int l = 0;

	if ( e == NULL ) {
		walker = challoc( sizeof(encoding) );

		if ( walker == NULL ) {
			goto cleanup;
		}

		walker->et = identity;
		walker->q = 1.0;

		append_encoding( &root, walker );

		return root;
	}

	char *t = strchr(e, ',');

	if ( t == NULL ) {
		root = parse_single_encoding( e );
	} else {
		while ( t ) {
			l = t - e;

			single = challoc( l + 1 );

			if ( single == NULL ) {
				goto cleanup;
			}

			memset( single, 0, l + 1);
			memcpy( single, e, l);

			e = t + 1;

			while( e[0] == ' ' && e[0] != 0x00 ) {
				e++;
			}

			accepted_encoding = parse_single_encoding( single );

			chfree(single);

			append_encoding( &root, accepted_encoding );

			t = strchr( e, ',');
		}

		accepted_encoding = parse_single_encoding( e );

		append_encoding( &root, accepted_encoding );
	}

	walker = challoc( sizeof(encoding) );

	if ( walker == NULL ) {
		goto cleanup;
	}

	walker->et = identity;
	walker->q = 1.0;

	append_encoding( &root, walker );

	return root;

cleanup:
	
	walker = root;

	while ( walker ) {
		encoding *t = walker->next;

		chfree(walker);
		walker = t;
	}

	return NULL;
}