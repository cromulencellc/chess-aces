#ifndef __URI_H__
#define __URI_H__


typedef struct uri_params {
	char *type;
	char *value;

	char *filename;
	int filesize;
	char *location;

	int error;

	struct uri_params *next;
} uri_params_t;

typedef struct uri {
	char *uri;

	uri_params_t *ups;
} uri_t;

uri_params_t *parse_params( char *param_string );
uri_params_t *parse_single_param( char *sp );
uri_t *parse_uri( char *uri );
void free_param( uri_params_t *p);

#endif