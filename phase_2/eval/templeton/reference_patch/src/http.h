#ifndef __HTTP_H__
#define __HTTP_H__


enum request_type {
	invalid,
	options,
	get,
	head,
	post,
	put,
	delete_e,
	trace,
	connect_e,
};

typedef struct field {
	char *name;
	char *value;
	struct field *next;
} field;

typedef struct request {
	char verb[16];
	enum request_type verb_rt;
	char *uri;

	int version_major;
	int version_minor;

	int content_length;
	char *content;

	field *field_list;
} request;

int handle_http_request( char *data);
request *parse_http_request( char *data );

#endif