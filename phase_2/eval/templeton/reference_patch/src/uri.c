#include "heap.h"
#include "uri.h"

#include <string.h>
#include <ctype.h>
#include <stdio.h>

char *convert_pcnts( char *uri )
{
	char *normd = NULL;
	int length = 0;
	int uri_index = 0;
	int normd_index = 0;

	char byte = 0;
	char upper;
	char lower;

	if ( uri == NULL ) {
		goto end;
	}

	length = strlen(uri);

	normd = challoc( length + 1);

	if ( normd == NULL ) {
		goto end;
	}

	memset(normd, 0, length + 1);

	while ( uri_index < length && normd_index < length && uri[uri_index] != 0x00 ){
		if ( uri[uri_index] != '%' ) {
			normd[normd_index++] = uri[uri_index++];
			continue;
		}

		if ( uri_index + 2 > length ) {
			normd[normd_index++] = uri[uri_index++];
			continue;
		}

		upper = tolower(uri[uri_index+1]);
		lower = tolower(uri[uri_index+2]);

		switch(upper) {
			case 'a' ... 'f':
				upper = upper - 0x57;
				break;
			case '0' ... '9':
				upper = upper - 0x30;
				break;
			default:
				normd[normd_index++] = uri[uri_index++];
				continue;
		};

		switch(lower) {
			case 'a' ... 'f':
				lower = lower - 0x57;
				break;
			case '0' ... '9':
				lower = lower - 0x30;
				break;
			default:
				normd[normd_index++] = uri[uri_index++];
				continue;
		};

		byte = (upper << 4) | lower;

		normd[normd_index++] = byte;
		uri_index += 3;
	}

end:
	return normd;
}

char *strip_slashes( char * uri )
{	
	char *norm = NULL;
	int length = 0;
	int uri_index = 0;
	int norm_index = 0;

	if ( uri == NULL ) {
		return uri;
	}

	length = strlen(uri);

	norm = challoc(length + 1);

	if ( norm == NULL ) {
		return norm;
	}

	memset( norm, 0, length + 1);

	while ( uri_index < length && norm_index < length && uri[uri_index] != 0x00 ) {
		if ( uri[uri_index] != '/') {
			norm[norm_index++] = uri[uri_index++];

			continue;
		}

		if ( norm_index == 0 ) {
			norm[norm_index++] = uri[uri_index++];

			continue;
		}

		if ( norm[norm_index-1] == '/') {
			uri_index++;

			continue;
		}

		norm[norm_index++] = uri[uri_index++];
	}

	return norm;
}

char *strip_dds( char * uri ) 
{
	char *norm = NULL;
	int length = 0;
	int uri_index = 0;
	int norm_index = 0;

	if ( uri == NULL ) {
		return uri;
	}

	length = strlen(uri);

	norm = challoc(length + 1);

	if ( norm == NULL ) {
		return norm;
	}

	memset( norm, 0, length + 1);

	while ( uri_index < length && norm_index < length && uri[uri_index] != 0x00 ) {

		if ( strncmp( uri + uri_index, "/../", 4) == 0 ) {
			
			if ( norm_index != 0 ) {
				norm_index -= 1;

				while ( norm_index > 0 && norm[norm_index] != '/') {
					norm_index -= 1;
				}
			}
			
			uri_index += 3;

			memset(norm + norm_index, 0, (length + 1) - norm_index );

			continue;
		}

		norm[norm_index++] = uri[uri_index++];
	}

	return norm;
}

char *strip_ds( char *uri)
{
	char *norm = NULL;
	int length = 0;
	int uri_index = 0;
	int norm_index = 0;

	if ( uri == NULL ) {
		return uri;
	}

	length = strlen(uri);

	norm = challoc(length + 1);

	if ( norm == NULL ) {
		return norm;
	}

	memset( norm, 0, length + 1);

	while ( uri_index < length && norm_index < length && uri[uri_index] != 0x00 ) {

		if ( strncmp( uri + uri_index, "/./", 3) == 0 ) {
			uri_index += 2;

			continue;
		}

		norm[norm_index++] = uri[uri_index++];
	}

	return norm;
}

char *escape_chars( char *uri )
{
	char *r = NULL;
	int length = 0;

	if ( uri == NULL ) {
		return NULL;
	}

	length = strlen(uri);

	r = challoc( (length * 2) + 1);

	if ( r == NULL) {
		return  NULL;
	}

	memset(r, 0, (length * 2) + 1);

	for ( int i = 0, j = 0; i < length; i++ ) {
		switch (uri[i]) {
			case '\'':
			case '$':
			case '\\':
			case '"':
				r[j] = '\\';
				j++;
				break;
			default:
				break;
		};

		r[j] = uri[i];
		j++;
	}

	return r;
}

void free_param( uri_params_t *p )
{
	if ( p == NULL ) {
		return;
	}

	if ( p->type ) {
		chfree(p->type);
	}

	if ( p->value ) {
		chfree(p->value);
	}

	if ( p->filename) {
		chfree(p->filename);
	}

	if ( p->location ) {
		chfree(p->location);
	}

	chfree(p);

	return;
}

uri_params_t *parse_single_param( char *sp )
{
	uri_params_t *upt = NULL;
	char *t = NULL;
	int length = 0;

	if ( sp == NULL ) {
		goto cleanup;
	}

	length = strlen(sp);

	t = strchr( sp, '=');

	if ( t == NULL ) {
		goto cleanup;
	}

	length = t - sp;

	if ( length == 0 ) {
		goto cleanup;
	}

	upt = challoc( sizeof(uri_params_t));

	if ( upt == NULL ) {
		goto cleanup;
	}

	upt->type = challoc( length + 1 );

	if ( upt->type == NULL ) {
		goto cleanup;
	}

	memset( upt->type, 0, length + 1);
	memcpy( upt->type, sp, length );

	t += 1;

	upt->value = convert_pcnts( t );

	if ( upt->value == NULL ) {
		goto cleanup;
	}

	upt->next = NULL;

	t = escape_chars( upt->type );

	if ( t == NULL ) {
		goto cleanup;
	}

	chfree( upt->type );

	upt->type = t;

	if ( upt->value[0] == '"' && upt->value[strlen(upt->value)-1] == '"') {
		memcpy(upt->value, upt->value + 1, strlen(upt->value) );

		upt->value[ strlen(upt->value) - 1] = 0x00;
	}

	t = escape_chars( upt->value );

	if ( t == NULL ) {
		goto cleanup;
	}

	chfree( upt->value );
	upt->value = t;

	return upt;

cleanup:
	if ( upt ) {
		if ( upt->type ) {
			chfree(upt->type);
		}

		if ( upt->value ) {
			chfree( upt->value );
		}

		chfree(upt);
	}	

	return NULL;
}

int add_linked_param( uri_params_t **root, uri_params_t *leaf )
{
	if ( root == NULL || leaf == NULL ) {
		return 1;
	}

	leaf->next = NULL;

	if ( *root == NULL ) {
		*root = leaf;

		return 0;
	}

	leaf->next = *root;
	*root = leaf;

	return 0;
}

uri_params_t *parse_params( char *param_string )
{
	uri_params_t *upt = NULL;
	uri_params_t *root = NULL;
	char *walker = NULL;
	char *amp = NULL;
	int length = 0;
	char *t = NULL;

	if ( param_string == NULL ) {
		return NULL;
	}

	walker = param_string;

	while ( walker ) {
		amp = strchr( walker, '&');

		if ( amp == NULL ) {
			upt = parse_single_param( walker );

			add_linked_param( &root, upt);

			return root;
		}

		length = amp - walker;

		t = challoc( length + 1 );

		if ( t == NULL ) {
			goto cleanup;
		}

		memset( t, 0, length + 1);

		memcpy(t, walker, length );

		upt = parse_single_param( t );

		chfree(t);

		add_linked_param(&root, upt);

		walker = amp + 1;
	}

	return root;
cleanup:
	while ( root ) {
		upt = root->next;

		if ( root->type) {
			chfree(root->type);
		}

		if ( root->value ) {
			chfree(root->value);
		}

		root = upt;
	}

	return NULL;
}

uri_t *parse_uri( char *uri )
{
	uri_t *u = NULL;
	char *new_uri = NULL;
	char *params = NULL;
	int length = 0;
	char *t = NULL;

	if ( uri == NULL ) {
		return NULL;
	}

	params = strchr(uri, '?');

	if ( params != NULL ) {
		length = params - uri;
	} else {
		length = strlen(uri);
	}

	new_uri = challoc(length + 1 );

	if ( new_uri == NULL ) {
		return NULL;
	} 

	memset( new_uri, 0, length + 1);
	memcpy( new_uri, uri, length );

	t = convert_pcnts( new_uri );

	chfree(new_uri);

	if ( t == NULL ) {
		return NULL;
	}

	new_uri = t;

	t = strip_slashes( new_uri );

	chfree(new_uri);

	if ( t ) {
		new_uri = t;
	} else {
		return NULL;
	}

	t = strip_ds( new_uri );

	chfree(new_uri);

	if ( t ) {
		new_uri = t;
	} else {
		return NULL;
	}

	t = strip_dds( new_uri );

	chfree(new_uri);

	if ( t ) {
		new_uri = t;
	} else {
		return NULL;
	}

	u = challoc( sizeof(uri_t));

	if ( u == NULL ) {
		chfree(new_uri);
		return NULL;
	}

	memset(u, 0, sizeof(uri_t));

	u->uri = new_uri;

	if ( params ) {
		u->ups = parse_params( params+1 );
	}

	return u;
}
