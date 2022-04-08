#define _GNU_SOURCE
#include "http.h"
#include "heap.h"
#include "string_funcs.h"
#include "uri.h"
#include "enc.h"
#include "range.h"
#include "mime_types.h"

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

field *parse_field( char * line )
{
	field *f = NULL;
	char *start = NULL;
	char *end = NULL;
	int length = 0;

	if ( line == NULL ) {
		return NULL;
	}

	start = line;
	end = strchr(start, ':');

	if ( end == NULL ) {
		return NULL;
	}

	if (start == end ) {
		return NULL;
	}

	f = challoc( sizeof(field));

	if ( f == NULL ) {
		goto cleanup;
	}

	memset(f, 0, sizeof(field));

	length = end - start;

	f->name = challoc( length + 1);

	if ( f->name == NULL ) {
		goto cleanup;
	}

	memset( f->name, 0, length + 1);
	memcpy( f->name, start, length);

	start = end + 1;

	while ( isblank(start[0]) ) {
		start++;
	}

	end = strstr(start, "\r\n");

	if ( end == NULL ) {
		goto cleanup;
	}

	if (start == end ) {
		goto cleanup;
	}

	length = end - start;

	f->value = challoc( length + 1);

	if ( f->value == NULL ) {
		goto cleanup;
	}

	memset( f->value, 0, length + 1);
	memcpy( f->value, start, length);

	return f;

cleanup:
	if ( f ) {
		if ( f->name ) {
			chfree(f->name);
		}

		if ( f->value ) {
			chfree(f->value);
		}

		chfree(f);
	}

	return NULL;
}

char * random_string( size_t length )
{
	char *str = challoc( length + 1 );
	int fd = 0;
	unsigned char c;

	if ( str == NULL ) {
		return NULL;
	}

	memset( str, 0, length + 1);

	fd = open("/dev/urandom", O_RDONLY);

	if ( fd <= 0 ) {
		chfree(str);

		return NULL;
	}

	for ( int i = 0; i < length; i++ ) {
		read( fd, &c, 1);

		c = (c % 26) + 0x61;

		str[i] = c;
	}

	close(fd);

	return str;
}

field *get_http_field( request *r, char *name )
{
	field * walker = NULL;

	if ( name == NULL || r == NULL ) {
		return NULL;
	}

	walker = r->field_list;

	while ( walker ) {
		if ( strcasecmp(walker->name, name) == 0 ) {
			break;
		}

		walker = walker->next;
	}

	return walker;
}

enum request_type translate_request_type( char *req )
{
	if ( req == NULL ) {
		return invalid;
	}

	if ( !strcasecmp( req, "options") ) {
		return options;
	} else if ( !strcasecmp( req, "get") ) {
		return get;
	} else if ( !strcasecmp( req, "head") ) {
		return head;
	} else if ( !strcasecmp( req, "post") ) {
		return post;
	} else if ( !strcasecmp( req, "put") ) {
		return put;
	} else if ( !strcasecmp( req, "delete") ) {
		return delete_e;
	} else if ( !strcasecmp( req, "trace") ) {
		return trace;
	} else if ( !strcasecmp( req, "connect") ) {
		return connect_e;
	}

	return invalid;
}

int send_error( int error_code, char *message )
{
	strbuf *response = NULL;
	strbuf *content = NULL;
	time_t current_time;

	if ( message == NULL ) {
		return 1;
	}

	content = init_string( "<html><head>\n<title>Error Response</title>\n</head>\n<body>\n<h1>Error response</h1><p>Error Code ");

	if ( content == NULL ) {
		return 1;
	}

	append_int( content, error_code);
	append_string(content, ". </p>\n<p>Message: ");
	append_string(content, message);
	append_string( content, "</p>\n</body>\n</html>");

	response = init_string( "HTTP/1.1 ");

	if ( response == NULL ) {
		free_string(content);
		return 1;
	}

	if ( expand_string( response, 1024 ) == 0) {
		free_string( content);
		free_string(response);
		return 1;
	}

	append_int( response, error_code );
	append_string( response, " " );
	append_string( response, message );

	append_string( response, "\r\n");

	append_string( response, "Server: CHESS\r\n");
	append_string( response, "Connection: Close\r\n");
	append_string( response, "Content-Type: text/html\r\n");

    current_time = time(NULL);

    append_string( response, "Date: ");
    append_string( response, asctime(localtime(&current_time)) );

    popchar(response);

    append_string( response, "\r\n");
    append_string( response, "Content-Length: ");
    append_int( response, get_length( content ) );
    append_string( response, "\r\n\r\n");

    append_strbuf( response, content);

    free_string(content);

    write(fileno(stdout), response->stream, response->length);

    free_string( response );

    return 0;
}

int send_basic_response( int code, char *message, field *additional_fields )
{
	strbuf *response = NULL;
	time_t current_time;

	if ( message == NULL ) {
		return 1;
	}

	response = init_string( "HTTP/1.1 ");

	if ( response == NULL ) {
		return 1;
	}

	if ( expand_string( response, 1024 ) == 0) {
		free_string(response);
		return 1;
	}

	append_int( response, code );
	append_string( response, " " );
	append_string( response, message );

	append_string( response, "\r\n");

	append_string( response, "Server: CHESS\r\n");
	append_string( response, "Connection: Close\r\n");
	append_string( response, "Accept-Ranges: bytes\r\n");

    current_time = time(NULL);

    append_string( response, "Date: ");
    append_string( response, asctime(localtime(&current_time)) );

    popchar(response);

    append_string(response, "\r\n");

    while ( additional_fields ) {
    	append_string( response, additional_fields->name);
    	append_string( response, ": ");
    	append_string( response, additional_fields->value);
    	append_string( response, "\r\n");

    	additional_fields = additional_fields->next;
    }

    append_string( response, "\r\n");

    write(fileno(stdout), response->stream, response->length);

    free_string( response );

    return 0;
}

struct tm *parse_date_string( char * data )
{
	struct tm * dt = NULL;
	char weekday[4];
	char month[4];
	char tmz[4];

	if ( data == NULL ) {
		return NULL;
	}

	dt = challoc( sizeof(struct tm) );

	if ( dt == NULL ) {
		return NULL;
	}

	memset( dt, 0, sizeof(struct tm) );

	sscanf(data, "%3s, %2d %3s %4d %2d:%2d:%2d %3s", 
		(char*)&weekday, &(dt->tm_mday), (char*)&month, &(dt->tm_year), &(dt->tm_hour), &(dt->tm_min), &(dt->tm_sec), (char*)&tmz);

	if ( strcmp( weekday, "Mon") == 0 ) {
		dt->tm_wday = 1;
	} else if ( strcmp( weekday, "Tue") == 0 ) {
		dt->tm_wday = 2;
	} else if ( strcmp( weekday, "Wed") == 0 ) {
		dt->tm_wday = 3;
	} else if ( strcmp( weekday, "Thu") == 0 ) {
		dt->tm_wday = 4;
	} else if ( strcmp( weekday, "Fri") == 0 ) {
		dt->tm_wday = 5;
	} else if ( strcmp( weekday, "Sat") == 0 ) {
		dt->tm_wday = 6;
	} else if ( strcmp( weekday, "Sun") == 0 ) {
		dt->tm_wday = 0;
	} else {
		chfree(dt);

		return NULL;
	}

	if ( strcmp( month, "Jan") == 0 ) {
		dt->tm_mon = 0;
	} else if ( strcmp( month, "Feb") == 0 ) {
		dt->tm_mon = 1;
	} else if ( strcmp( month, "Mar") == 0 ) {
		dt->tm_mon = 2;
	} else if ( strcmp( month, "Apr") == 0 ) {
		dt->tm_mon = 3;
	} else if ( strcmp( month, "May") == 0 ) {
		dt->tm_mon = 4;
	} else if ( strcmp( month, "Jun") == 0 ) {
		dt->tm_mon = 5;
	} else if ( strcmp( month, "Jul") == 0 ) {
		dt->tm_mon = 6;
	} else if ( strcmp( month, "Aug") == 0 ) {
		dt->tm_mon = 7;
	} else if ( strcmp( month, "Sep") == 0 ) {
		dt->tm_mon = 8;
	} else if ( strcmp( month, "Oct") == 0 ) {
		dt->tm_mon = 9;
	} else if ( strcmp( month, "Nov") == 0 ) {
		dt->tm_mon = 10;
	} else if ( strcmp( month, "Dec") == 0 ) {
		dt->tm_mon = 11;
	} else {
		chfree(dt);

		return NULL;
	}

	if ( strcmp(tmz, "GMT") != 0 ) {
		chfree(dt);

		return NULL;
	}

	// This is necessary
	dt->tm_year -= 1900;

	/// Sanity checks
	if ( dt->tm_mday > 31 || dt->tm_mday < 1) {
		chfree(dt);
		return NULL;
	}

	if ( dt->tm_hour > 23 || dt->tm_hour < 0) {
		chfree(dt);
		return NULL;
	}

	if ( dt->tm_min > 59 || dt->tm_min < 0) {
		chfree(dt);
		return NULL;
	}

	if ( dt->tm_sec > 59 || dt->tm_sec < 0) {
		chfree(dt);
		return NULL;
	}

	return dt;
}

int combine_encoding_headers( request * r)
{
	field *prev = NULL;
	field *walker = NULL;
	field *aef = NULL;

	char *new_value = NULL;

	int length = 0;

	if ( r == NULL ) {
		return 1;
	}

	walker = r->field_list;

	walker = r->field_list;

	while ( walker ) {
		if ( strcasecmp(walker->name, "Accept-Encoding") == 0) {
			if (prev != NULL ) {
				prev->next = walker->next;
			}

			if ( aef == NULL ) {
				aef = walker;

			} else {
				length = strlen(walker->value) + strlen(aef->value) + 3;

				new_value = challoc( length );

				if ( new_value == NULL ) {
					return 1;
				}

				memset( new_value, 0, length );

				strcpy( new_value, aef->value  );
				strcat( new_value, ", ");
				strcat( new_value, walker->value);

				chfree( walker->name);
				chfree( walker->value);
				chfree( walker );

				chfree( aef->value );

				aef->value = new_value;
			}

			walker = walker->next;
		} else {
			prev = walker;
			walker = walker->next;
		}

	}

	if ( aef ) {
		aef->next = r->field_list;
		r->field_list = aef;
	}
	

	return 0;
}

request *parse_http_request( char *data )
{
	request *r = NULL;

	if ( data == NULL ) {
		goto cleanup;
	}

	r = challoc( sizeof(request) );

	if ( r == NULL ) {
		send_error( 500, "Internal Server Error");

		goto cleanup;
	}

	memset(r, 0, sizeof(request) );

	int length = 0;
	char *current_loc = data;
	char *temp = NULL;
	char *field_token = NULL;
	field *new_field = NULL;
	char prot[5];

	temp = strchr( current_loc, ' ');

	if ( temp == NULL ) {
		send_error( 500, "Internal Server Error");

		goto cleanup;
	}

	if ( temp - current_loc >= 16 ) {
		send_error( 400, "Bad Request");

		goto cleanup;
	}

	memcpy( r->verb, current_loc, temp - current_loc );

	r->verb_rt = translate_request_type( (char*)&(r->verb) );

	if ( r->verb_rt == invalid ) {
		send_error( 501, "Not Implemented");

		goto cleanup;
	}

	current_loc = temp + 1;

	temp = strchr( current_loc, ' ');

	if ( temp == NULL ) {
		send_error( 400, "Bad Request");

		goto cleanup;
	}

	length = temp - current_loc;

	r->uri = challoc( length + 1 );

	if ( r->uri == NULL ) {
		send_error( 500, "Internal Server Error");

		goto cleanup;
	}

	memcpy( r->uri, current_loc, length );

	current_loc = temp + 1;

	temp = strstr(current_loc, "\r\n");

	if ( temp == NULL ) {
		send_error( 400, "Bad Request");

		goto cleanup;
	}

	sscanf( current_loc, "%4s/%d.%d", prot, &(r->version_major), &(r->version_minor));

	if ( strcasecmp( prot, "HTTP") ) {
		send_error( 400, "Bad Request");

		goto cleanup;
	}

	if ( r->version_major == 1 ) {
		if ( r->version_minor != 1 ) {
			send_error( 505, "HTTP Version Not Supported");

			goto cleanup;
		}
	} else if ( r->version_major == 2 ) {
		if ( r->version_minor != 0 ) {
			send_error( 505, "HTTP Version Not Supported");

			goto cleanup;
		}
	} else {
		send_error( 505, "HTTP Version Not Supported");

		goto cleanup;
	}

	current_loc = temp + 2;

	temp = strstr(current_loc, "\r\n");

	while ( temp ) {
		length = temp - current_loc;

		if ( length == 0 ) {
			break;
		}

		char *line = challoc( length + 1 );

		if ( line == NULL ) {
			send_error( 500, "Internal Server Error");

			goto cleanup;
		}

		memset( line, 0, length + 1);

		memcpy( line, current_loc, length );

		field_token = strchr( line, ':');

		if ( field_token == NULL ) {
			send_error( 400, "Bad Request");

			chfree(line);

			goto cleanup;
		}

		length = field_token - line;

		if ( length == 0 ) {
			send_error( 400, "Bad Request");

			chfree(line);
			goto cleanup;
		}

		new_field = challoc( sizeof(field) );

		if ( new_field == NULL ) {
			send_error( 500, "Internal Server Error");

			chfree(line);
			goto cleanup;
		}

		memset(new_field, 0, sizeof(field) );

		new_field->name = challoc(length + 1);

		if ( new_field->name == NULL ) {
			send_error( 500, "Internal Server Error");

			chfree(line);
			chfree(new_field);

			goto cleanup;
		}

		memset( new_field->name, 0, length + 1);

		memcpy( new_field->name, line, length);

		if ( line[length + 1] == ' ') {
			length += 2;
		} else {
			length += 1;
		}

		int val_length = strlen(line + length);

		if ( val_length == 0 ) {
			send_error( 400, "Bad Request");

			chfree( new_field->name );
			chfree(line);
			chfree(new_field);

			goto cleanup;
		}

		new_field->value = challoc(val_length + 1);

		if ( new_field->value == NULL ) {
			send_error( 500, "Internal Server Error");

			chfree(line);
			chfree( new_field->name );
			chfree(new_field);

			goto cleanup;
		}

		memset( new_field->value, 0, val_length + 1);

		memcpy( new_field->value, line + length, val_length);

		new_field->next = r->field_list;
		r->field_list = new_field;

		current_loc = temp + 2;

		temp = strstr(current_loc, "\r\n");

		chfree(line);
	}

	if ( combine_encoding_headers( r ) ) {
		send_error( 500, "Internal Server Error");

		goto cleanup;
	}

	field *walker = r->field_list;

	field *f = get_http_field( r, "Expect");

	if ( f ) {
		if ( strcasecmp(f->value, "100-continue") == 0) {
			send_basic_response( 100, "Continue", NULL);
		} else {
			send_error( 417, "Expectation Failed");

			goto cleanup;
		}
	}

	walker = get_http_field(r, "Content-Length");

	if ( walker != NULL ) {
		r->content_length = atoi(walker->value);

		r->content = challoc(r->content_length + 1);

		if ( r->content == NULL ) {
			goto cleanup;
		}

		memset(r->content, 0, r->content_length + 1);

		int recvd = 0;
		int result = 0;

		while (recvd < r->content_length ) {
			result = read(fileno(stdin), r->content + recvd, r->content_length - recvd);

			if ( result <= 0 ) {
				break;
			}

			recvd += result;			
		}
		

		if ( result <= 0 ) {
			goto cleanup;
		}

		r->content_length = recvd;
	}

	return r;

cleanup:
	if ( r ) {
		if ( r->uri ) {
			chfree(r->uri);
		}

		if ( r->content ) {
			chfree(r->content);
		}

		chfree(r);
	}

	return NULL;
}

int host_field_count( request *r )
{
	field * walker = NULL;
	int count = 0;

	if ( r == NULL ) {
		return 0;
	}

	walker = r->field_list;

	while ( walker ) {
		if ( strcasecmp(walker->name, "Host") == 0 ) {
			count += 1;
		}

		walker = walker->next;
	}

	return count;
}

int send_file( request *r, char *name, encoding *encs, uri_params_t *params, int sendit )
{
	field *range = NULL;
	byte_range *t = NULL;
	byte_range *rl = NULL;

	struct stat st;
	strbuf *content = NULL;
	strbuf *content_type = NULL;
	strbuf *response = NULL;

	char * boundary = NULL;
	char * filedata = NULL;
	char * mimetype = NULL;
	char *temp_content = NULL;

	time_t current_time;

	int ispartial = 0;
	int fd = 0;
	int retval = 1;

	if ( r == NULL || name == NULL  ) {
		goto cleanup;
	}

	if ( stat( name, &st ) ) {
		goto cleanup;
	}

	filedata = challoc( st.st_size );

	if ( filedata == NULL ) {
		goto cleanup;
	}

	fd = open( name, O_RDONLY );

	if ( fd <= 0 ) {
		goto cleanup;
	}

	read( fd, filedata, st.st_size );
	close(fd);

	range = get_http_field( r, "Range");

	rl = NULL;

	mimetype = extension_to_content_type(name);
	content_type = init_string( mimetype );

	if ( content_type == NULL ) {
		goto cleanup;
	}

	if ( strcmp(mimetype, "application/x-httpd-php") == 0) {
		strbuf *post_php = init_string("<?php ");
		char *varname = NULL;

		if ( post_php == NULL ) {
			goto cleanup;
		}

		if ( r->verb_rt == get ) {
			append_string(post_php, "$_SERVER[\"REQUEST_METHOD\"] = \"GET\";\n");
			varname = "$_GET";
		} else if ( r->verb_rt == post ) {
			append_string(post_php, "$_SERVER[\"REQUEST_METHOD\"] = \"POST\";\n");
			varname = "$_POST";
		} else {

			free_string(post_php);

			goto cleanup;
		}

		while(params) {
			if ( params->filename ) {
				char tmpphp[] = "/tmp/chess_XXXXXX";

				int php_fd = mkstemp(tmpphp);

				if ( php_fd <= 0 ) {
					free_string(post_php);

					goto cleanup;
				}

				write( php_fd, params->value, params->filesize );
				close(php_fd);

				append_string(post_php, "$_FILES[\"");
				append_string(post_php, params->type );
				append_string(post_php, "\"][\"name\"] = \"");
				append_string(post_php, params->filename );
				append_string(post_php, "\";\n");

				append_string(post_php, "$_FILES[\"");
				append_string(post_php, params->type );
				append_string(post_php, "\"][\"tmp_name\"] = \"");
				append_string(post_php, tmpphp );
				append_string(post_php, "\";\n");

				append_string(post_php, "$_FILES[\"");
				append_string(post_php, params->type );
				append_string(post_php, "\"][\"size\"] = ");
				append_int(post_php, params->filesize );
				append_string(post_php, ";\n");

				append_string(post_php, "$_FILES[\"");
				append_string(post_php, params->type );
				append_string(post_php, "\"][\"error\"] = ");
				append_int(post_php, 0 );
				append_string(post_php, ";\n");

				append_string(post_php, "$_FILES[\"");
				append_string(post_php, params->type );
				append_string(post_php, "\"][\"type\"] = \"");
				append_string(post_php, extension_to_content_type(params->filename) );
				append_string(post_php, "\";\n");
			} else {
				append_string(post_php, varname);
				append_string(post_php, "['");
				append_string(post_php, params->type);
				append_string(post_php, "']=\"");
				append_string(post_php, params->value);
				append_string(post_php, "\"; ");
			}
			

			params = params->next;
		}

		append_string(post_php, " ?>\n");
		append_octets(post_php, filedata, st.st_size );

		char tmpfile[] = "/tmp/chess_XXXXXX";

		int fd = mkstemp(tmpfile);

		if ( fd <= 0 ) {
			free_string(post_php);

			goto cleanup;
		}

		write(fd, post_php->stream, post_php->length);
		close(fd);

		free_string(post_php);
		post_php = NULL; 

		char command[256];

		snprintf(command, 256, "/usr/bin/php %s > %s.php", tmpfile, tmpfile);

		system(command);

		snprintf(command, 256, "%s.php", tmpfile);

		char *php_file;

		if ( stat( command, &st ) ) {
			goto cleanup;
		}

		php_file = challoc( st.st_size );

		if ( php_file == NULL ) {
			goto cleanup;
		}

		fd = open(command, O_RDONLY);

		if ( fd <= 0 ) {
			goto cleanup;
		}

		read( fd, php_file, st.st_size );
		close(fd);

		chfree(filedata);
		filedata = php_file;
	}

	if ( range && r->verb_rt == get ) {
		rl = parse_ranges(range->value);

		rl = delete_invalid( rl, st.st_size);

		if ( rl != NULL ) {
			ispartial = 1;

			boundary = random_string( 25 );

			if ( boundary == NULL ) {
				goto cleanup;
			}

			content = init_string( "" );

			if ( expand_string( content, 1024 ) == 0 ) {
				goto cleanup;
			}

			t = rl;

			while ( t ) {
				append_string(content, "--" );
				append_string(content, boundary );
				append_string(content, "\r\n");

				append_string(content, "Content-Type: ");
				append_strbuf(content, content_type );
				append_string(content, "\r\n");

				append_string( content, "Content-Range: bytes " );
				append_int( content, t->start );
				append_string( content, "-");
				append_int( content, t->end );

				append_string( content, "/");
				append_int( content, st.st_size);
				append_string(content, "\r\n\r\n");

				if ( t->end == -1 ) {
					t->end = st.st_size;
				}

				int l = (t->end - t->start ) + 1;

				temp_content = challoc( l + 1);

				if ( temp_content == NULL ) {
					goto cleanup;
				}

				memset( temp_content, 0, l);
				memcpy( temp_content, filedata + t->start, l);

				append_octets(content, temp_content, l);
				append_string(content, "\r\n");

				chfree(temp_content);

				t = t->next;
			}

			append_string(content, boundary );
			append_string(content, "--\r\n");
		} else {
			content = init_octets( filedata, st.st_size );

			if ( content == NULL ) {
				goto cleanup;
			}
		}
	} else {
		content = init_octets( filedata, st.st_size );

		if ( content == NULL ) {
			goto cleanup;
		}
	}


	if ( ispartial ) {
		response = init_string("HTTP/1.1 206 Partial Content\r\n");

		if ( response == NULL ) {
			goto cleanup;
		}

		append_string( response, "Content-Type: ");
    	append_string( response, "multipart/byteranges; boundary=");
    	append_string( response, boundary);
    	append_string( response, "\r\n");
	} else {
		response = init_string("HTTP/1.1 200 OK\r\n");

		if ( response == NULL ) {
			goto cleanup;
		}

		append_string( response, "Content-Type: ");
		append_string( response, mimetype );
		append_string( response, "\r\n");
	}

	current_time = time(NULL);

    append_string( response, "Date: ");
    append_string( response, asctime(localtime(&current_time)) );

    popchar(response);

    append_string(response, "\r\n");

    struct tm *file_mod_time = localtime( &st.st_mtime );
	time_t time0 = mktime(file_mod_time);

    append_string( response, "Last-Modified: ");
    append_string( response, ctime(&time0) );
    popchar(response);

    append_string( response, "\r\n");

    append_string( response, "Server: CHESS\r\n");

    append_string( response, "Content-Length: ");
    append_int( response, get_length(content) );

    append_string( response, "\r\n\r\n");

    if ( sendit ) {
    	append_strbuf( response, content);
    }
    
    write(fileno(stdout), response->stream, response->length);

    retval = 0;

    free_string(response);

cleanup:
	if ( filedata ) {
		chfree(filedata );
	}

	t = rl;

	while (rl) {
		t = rl->next;
		chfree(rl);
		rl = t;
	}

	if ( boundary ) {
		chfree( boundary );
	}

	if ( content ) {
		free_string( content );
	}

	if (content_type) {
		free_string(content_type);
	}

	return retval;
}

uri_params_t *parse_multipart_form( char *data, int length )
{
	field *f = NULL;
	field *root = NULL;

	char *start = NULL;
	char *end = NULL;
	char *field_data = NULL;
	char *temp_param = NULL;

	uri_params_t *ups = NULL;
	uri_params_t *fileparam = NULL;

	int slen = 0;

	if ( data == NULL || length <= 0 ) {
		return NULL;
	}

	start = data;
	end = strstr(start, "\r\n");

	if ( end == NULL ) {
		goto cleanup;
	}

	while ( start != end ) {
		f = parse_field( start );

		if ( f == NULL ) {
			goto cleanup;
		}

		f->next = root;
		root = f;

		start = end + 2;

		end = strstr(start, "\r\n");

		if ( end == NULL ) {
			goto cleanup;
		}
	}

	start += 2;

	f = root;

	while (f) {
		if ( strcasecmp(f->name, "Content-Disposition") == 0 ) {
			break;
		}

		f = f->next;
	}

	if ( f == NULL ) {
		goto cleanup;
	}

	field_data = f->value;

	if ( strncasecmp(field_data, "form-data;", 10 ) ) {
		goto cleanup;
	}
		
	field_data += 10;

	while (isblank(field_data[0]) ) {
		field_data++;
	}

	end = strchr( field_data, ';');

	if ( end == NULL ) {
		slen = strlen(field_data);
	} else {
		slen = end - field_data;
	}

	temp_param = challoc( slen + 1);

	if ( temp_param == NULL ) {
		goto cleanup;
	}

	memset(temp_param, 0, slen + 1);
	memcpy(temp_param, field_data, slen);

	ups = parse_single_param( temp_param );

	chfree(temp_param);

	if ( ups == NULL ) {
		goto cleanup;
	}

	if ( end ) {
		end += 1;

		while ( isblank(end[0]) ) {
			end++;
		}

		fileparam = parse_single_param( end );

		if ( fileparam == NULL ) {
			goto cleanup;
		}

		if ( strcmp(fileparam->type, "filename") ) {
			goto cleanup;
		}

		ups->filename = fileparam->value;

		slen = length - (start - data);

		slen -= 2;

		chfree(ups->type);

		ups->type = ups->value;

		ups->value = challoc( slen );

		if ( ups->value == NULL ) {
			goto cleanup;
		}

		memcpy( ups->value, start, slen );

		ups->filesize = slen;

		chfree(fileparam->type);
		chfree(fileparam);
		fileparam = NULL;
	} else {
		slen = length - (start - data);

		slen -= 2;

		if ( slen <= 0 ) {
			goto cleanup;
		}

		chfree(ups->type);
		ups->type = ups->value;

		ups->value = challoc( slen + 1);

		if ( ups->value == NULL ) {
			goto cleanup;
		}

		memset( ups->value, 0, slen + 1 );

		memcpy( ups->value, start, slen );
	}

	goto end;

cleanup:
	if ( fileparam ) {
		if ( fileparam->type ) {
			chfree(fileparam->type);
		}

		if ( fileparam->value) {
			chfree(fileparam->value);
		}

		chfree(fileparam);
		fileparam = NULL;
	}

	if ( ups ) {
		if ( ups->type ) {
			chfree(ups->type);
		}

		if ( ups->value) {
			chfree(ups->value);
		}

		chfree(ups);
		ups = NULL;
	}

	while ( root ) {
		f = root->next;

		if ( root->name ) {
			chfree(root->name);
		}

		if ( root->value ) {
			chfree( root->value);
		}

		chfree(root);

		root = f;
	}

end:
	return ups;
}

uri_params_t *parse_post_params( request *r )
{
	field *f = NULL;

	char * boundary = NULL;
	char *start = NULL;
	char *end = NULL;
	char *temp_param = NULL;

	int length = 0;
	int haystack_len = 0;
	int needle_len = 0;

	uri_params_t *root = NULL;
	uri_params_t *ups = NULL;

	if ( r == NULL ) {
		return NULL;
	}

	if ( r->content == NULL ) {
		return NULL;
	}

	f = get_http_field(r, "Content-Type");

	if ( f == NULL ) {
		return parse_params( r->content );
	}

	if ( strncasecmp(f->value, "multipart/form-data", 19) ) {
		return parse_params( r->content );
	}

	start = strchr( f->value, ';');

	if ( start == NULL ) {
		return NULL;
	}

	start += 1;

	while ( isblank(start[0]) ) {
		start++;
	}

	if ( strncasecmp(start, "boundary=", 9) ) {
		return NULL;
	}

	start += 9;

	length = strlen(start);

	boundary = challoc( length + 3);

	if ( boundary == NULL ) {
		return NULL;
	}

	memset(boundary, 0, length + 3);
	boundary[0] = '-';
	boundary[1] = '-';

	memcpy(boundary+2, start, length);

	needle_len = length;

	haystack_len = r->content_length;
	start = r->content;

	start = memmem(start, haystack_len, boundary, needle_len);

	while ( start ) {
		start += strlen(boundary) + 2;

		haystack_len = r->content_length - ( start - r->content );

		end = memmem(start, haystack_len, boundary, needle_len);

		if ( end ) {
			length = end - start;
		} else {
			length = haystack_len;
		}

		if ( length <= 0 ) {
			chfree(boundary);

			goto end;
		}


		temp_param = challoc( length + 1 );

		if ( temp_param == NULL ) {
			goto cleanup;
		}

		memset( temp_param, 0, length + 1);
		memcpy( temp_param, start, length );

		ups = parse_multipart_form( temp_param, length );

		chfree(temp_param);

		if ( ups ) {
			ups->next = root;
			root = ups;
		}

		start = end;
	}

	goto end;

cleanup:
	if ( boundary ) {
		chfree(boundary);
	}

	while ( root ) {
		ups = root->next;

		if ( root->type ) {
			chfree(root->type);
		}

		if ( root->value ) {
			chfree( root->value);
		}

		chfree(root);

		root = ups;
	}

	root = NULL;

end:
	return root;

}

int handle_http_v1_connect( request *r )
{
	uri_t *u = NULL;
	int port = 80;

	char *ip = NULL;
	char *end = NULL;

	struct sockaddr_in sa;
	int length = 0;

	int fd;
	fd_set read_fds;
	fd_set except_fds;

	int result = 0;
	int read_bytes = 0;

	char data[1024];

	if ( r == NULL ) {
		goto cleanup;
	}

	if ( host_field_count( r ) != 1) {
		send_error( 400, "Bad Request");

		return 0;
	}

	u = parse_uri( r->uri );

	if ( u == NULL ) {
		send_error( 500, "Internal Server Error");

		return 1;
	}

	end = strchr(u->uri, ':');

	if ( end ) {
		port = atoi( end + 1);
	}

	length = end - u->uri;

	ip = challoc( length + 1);

	if ( ip == NULL ) {
		send_error( 500, "Internal Server Error");

		return 1;
	}

	memset(ip, 0, length + 1 );
	memcpy( ip, u->uri, length );

	if ( inet_pton(AF_INET, ip, &sa) == 0 ) {
		send_error( 400, "Bad Request");

		goto cleanup;
	}

	fd = socket(AF_INET, SOCK_STREAM, 0);

	if ( fd == -1 ) {
		send_error(500, "Internal Server Error");

		goto cleanup;
	}

	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);

	if ( connect(fd, (struct sockaddr *)&sa, sizeof(sa)) != 0 ) {
		send_error(504, "Gateway Timeout");

		goto cleanup;
	}

	send_basic_response(200, "OK", NULL);

	while ( 1 ) {
		FD_ZERO(&read_fds);
		FD_ZERO(&except_fds);

		FD_SET(fileno(stdin), &read_fds);
		FD_SET(fd, &read_fds);

		FD_SET(fileno(stdin), &except_fds);
		FD_SET(fd, &except_fds);

		result = select(fd + 1, &read_fds, NULL, &except_fds, NULL);

		if ( result <= 0 ) {
			send_error(504, "Gateway Timeout");

			close(fd);
			goto cleanup;
		}

		if (FD_ISSET(fileno(stdin), &read_fds)) {
			memset(data, 0, 1024);

			read_bytes = read(fileno(stdin), data, 1024);

			if ( read_bytes <= 0 ) {

				send_error(504, "Gateway Timeout");

				close(fd);

				goto cleanup;
			}

			if ( write( fd, data, read_bytes) <= 0 ) {

				send_error(504, "Gateway Timeout");

				close(fd);

				goto cleanup;
			}
        }

        if (FD_ISSET(fd, &read_fds)) {
        	memset(data, 0, 1024);

        	read_bytes = read(fd, data, 1024);

        	if ( read_bytes <= 0 ) {
        		send_error(504, "Gateway Timeout");

        		close(fd);

        		goto cleanup;
        	}

        	if ( write( fileno(stdin), data, read_bytes) <= 0 ) {
        		send_error(504, "Gateway Timeout");

        		close(fd);

        		goto cleanup;

        	}
        }

        if (FD_ISSET(fileno(stdin), &except_fds)) {
        	send_error(504, "Gateway Timeout");

        	close(fd);

        	goto cleanup;
        }

        if (FD_ISSET(fd, &except_fds)) {
        	send_error(504, "Gateway Timeout");

        	close(fd);

        	goto cleanup;
        }
	}

cleanup:

	if ( ip ) {
		chfree(ip);
	}

	return 0;
}

int handle_http_v1_post( request *r )
{
	uri_t *u = NULL;
	int length = 0;
	char *filename = NULL;
	struct stat st;
	field *f = NULL;
	encoding * encs = NULL;

	uri_params_t *content_params = NULL;
	uri_params_t *walker = NULL;

	if ( r == NULL ) {
		goto cleanup;
	}

	if ( host_field_count( r ) != 1) {
		send_error( 400, "Bad Request");

		return 0;
	}

	// Fix up the uri
	u = parse_uri( r->uri );

	if ( u == NULL ) {
		send_error( 500, "Internal Server Error");

		return 1;
	}

	if ( u->uri[0] != '/' ) {
		send_error( 400, "Bad Request");

		chfree(u->uri);
		chfree(u);

		return 0;
	}

	length = 5 + strlen(u->uri);

	filename = challoc( length + 1 );

	if ( filename == NULL ) {
		send_error( 500, "Internal Server Error");

		chfree(u->uri);
		chfree(u);

		return 1;
	}

	memset( filename, 0, length + 1);

	strcpy( filename, "/data");
	strcat( filename, u->uri );

	if ( stat( filename, &st ) ) {
		send_error( 404, "Not Found");

		chfree(filename);

		chfree(u->uri);
		chfree(u);

		return 0;
	}

	if ( (st.st_mode & S_IFREG ) == 0 ) {
		send_error( 403, "Forbidden");

		chfree(filename);
		chfree(u->uri);
		chfree(u);

		return 0;
	}

	f = get_http_field(r, "Accept-Encoding");

	if ( f ) {
		encs = parse_encodings( f->value );
	} else {
		encs = parse_encodings( NULL );
	}

	f = get_http_field(r, "Content-Type");

	if ( r->content ) {
		content_params = parse_post_params( r );

		uri_params_t *form_params = content_params;

		while ( form_params ) {
			walker = form_params->next;

			form_params->next = u->ups;
			u->ups = form_params;

			form_params = walker;
		}
	}

	send_file( r, filename, encs, u->ups, 1 );

	chfree(filename);
	chfree(u->uri);
	chfree(u);

	encoding *t = NULL;

	while ( encs ) {
		t = encs->next;

		chfree(encs);
		encs = t;
	}

cleanup:
	return 1;
}

int handle_http_v1_get( request *r )
{
	struct stat st;
	char *filename = NULL;
	int length = 0;
	encoding *encs = NULL;
	field *f = NULL;
	uri_t *u = NULL;

	if ( r == NULL ) {
		return 1;
	}

	if ( host_field_count( r ) != 1) {
		send_error( 400, "Bad Request");

		return 0;
	}

	u = parse_uri( r->uri );

	if ( u == NULL ) {
		send_error( 500, "Internal Server Error");

		return 1;
	}

	if ( u->uri[0] != '/' ) {
		send_error( 400, "Bad Request");

		chfree(u->uri);
		chfree(u);

		return 0;
	}

	length = 5 + strlen(u->uri);

	filename = challoc( length + 1 );

	if ( filename == NULL ) {
		send_error( 500, "Internal Server Error");

		chfree(u->uri);
		chfree(u);

		return 1;
	}

	memset( filename, 0, length + 1);

	strcpy( filename, "/data");
	strcat( filename, u->uri );

	if ( stat( filename, &st ) ) {
		send_error( 404, "Not Found");

		chfree(filename);

		chfree(u->uri);
		chfree(u);

		return 0;
	}

	if ( (st.st_mode & S_IFREG ) == 0 ) {
		send_error( 403, "Forbidden");

		chfree(filename);

		chfree(u->uri);
		chfree(u);

		return 0;
	}

	f = get_http_field(r, "If-Modified-Since");

	if ( f ) {
		struct tm *dt = parse_date_string(f->value);

		if ( dt != NULL ) {
			time_t time1 = mktime(dt);

			struct tm *file_mod_time = localtime( &st.st_mtime );
			time_t time0 = mktime(file_mod_time);

			double delta = difftime( time1, time0);

			chfree(dt);

			if ( delta >= 0.0 ) {
				field lmf;

				lmf.name = "Last-Modified";
				lmf.value = ctime(&time0);
				lmf.next = NULL;

				send_basic_response( 304, "Not Modified", &lmf);
				chfree(filename);

				return 0;
			}
		}
	}
	
	/// Check Encoding Types
	f = get_http_field(r, "Accept-Encoding");

	if ( f ) {
		encs = parse_encodings( f->value );
	} else {
		encs = parse_encodings( NULL );
	}

	send_file( r, filename, encs, u->ups, 1 );

	chfree(filename);
	chfree(u->uri);
	chfree(u);

	encoding *t = NULL;

	while ( encs ) {
		t = encs->next;

		chfree(encs);
		encs = t;
	}

	return 0;
}

int handle_http_v1_head( request *r )
{
	struct stat st;
	char *filename = NULL;
	int length = 0;
	encoding *encs = NULL;
	field *f = NULL;
	uri_t *u = NULL;

	if ( r == NULL ) {
		return 1;
	}

	if ( host_field_count( r ) != 1) {
		send_error( 400, "Bad Request");

		return 0;
	}

	u = parse_uri( r->uri );

	if ( u == NULL ) {
		send_error( 500, "Internal Server Error");

		return 1;
	}

	if ( u->uri[0] != '/' ) {
		send_error( 400, "Bad Request");
		chfree(u->uri);
		chfree(u);

		return 0;
	}

	length = 5 + strlen(u->uri);

	filename = challoc( length + 1 );

	if ( filename == NULL ) {
		send_error( 500, "Internal Server Error");

		chfree(u->uri);
		chfree(u);

		return 1;
	}

	memset( filename, 0, length + 1);

	strcpy( filename, "/data");
	strcat( filename, u->uri );

	if ( stat( filename, &st ) ) {
		send_error( 404, "Not Found");

		chfree(filename);
		chfree(u->uri);
		chfree(u);

		return 0;
	}

	if ( (st.st_mode & S_IFREG ) == 0 ) {
		send_error( 403, "Forbidden");

		chfree(filename);
		chfree(u->uri);
		chfree(u);

		return 0;
	}

	f = get_http_field(r, "If-Modified-Since");

	if ( f ) {
		struct tm *dt = parse_date_string(f->value);

		if ( dt != NULL ) {
			time_t time1 = mktime(dt);

			struct tm *file_mod_time = localtime( &st.st_mtime );
			time_t time0 = mktime(file_mod_time);

			double delta = difftime( time1, time0);

			chfree(dt);

			if ( delta >= 0.0 ) {
				field lmf;

				lmf.name = "Last-Modified";
				lmf.value = ctime(&time0);
				lmf.next = NULL;

				send_basic_response( 304, "Not Modified", &lmf);
				chfree(filename);

				return 0;
			}
		}
	}
	
	f = get_http_field(r, "Accept-Encoding");

	if ( f ) {
		encs = parse_encodings( f->value );
	} else {
		encs = parse_encodings( NULL );
	}

	send_file( r, filename, encs, u->ups, 0 );

	chfree(filename);
	chfree(u->uri);
	chfree(u);

	encoding *t = NULL;

	while ( encs ) {
		t = encs->next;

		chfree(encs);
		encs = t;
	}

	return 0;
}

int handle_http_v1_put( request *r )
{
	uri_t *u = NULL;
	int length = 0;
	char *filename = NULL;
	struct stat st;
	field *f = NULL;
	encoding * encs = NULL;
	int exists = 0;

	uri_params_t *content_params = NULL;
	uri_params_t *walker = NULL;

	char * filedata = NULL;
	int file_length = 0;

	char * type = NULL;

	if ( r == NULL ) {
		goto cleanup;
	}

	if ( host_field_count( r ) != 1) {
		send_error( 400, "Bad Request");

		return 0;
	}

	u = parse_uri( r->uri );

	if ( u == NULL ) {
		send_error( 500, "Internal Server Error");

		return 1;
	}

	if ( u->uri[0] != '/' ) {
		send_error( 400, "Bad Request");

		chfree(u->uri);
		chfree(u);

		return 0;
	}

	length = 19 + strlen(u->uri);

	filename = challoc( length + 1 );

	if ( filename == NULL ) {
		send_error( 500, "Internal Server Error");

		chfree(u->uri);
		chfree(u);

		return 1;
	}

	memset( filename, 0, length + 1);

	strcpy( filename, "/data");
	strcat( filename, u->uri );

	type = extension_to_content_type( filename );

	if ( strcmp( type, "application/x-httpd-php") == 0 ) {
		send_error( 403, "Forbidden");

		chfree(filename);
		chfree(u->uri);
		chfree(u);

		return 0;
	}

	if ( stat( filename, &st ) ) {
		exists = 0;
	} else {
		exists = 1;
	}

	if ( (st.st_mode & S_IFREG ) == 0 && exists ) {
		send_error( 403, "Forbidden");

		chfree(filename);
		chfree(u->uri);
		chfree(u);

		return 0;
	}

	f = get_http_field(r, "Accept-Encoding");

	if ( f ) {
		encs = parse_encodings( f->value );
	} else {
		encs = parse_encodings( NULL );
	}

	f = get_http_field(r, "Content-Type");

	if ( f ) {
		if ( strncasecmp(f->value, "multipart/form-data", 19) == 0 ) {
			content_params = parse_post_params( r );

			if ( content_params == NULL ) {
				send_error( 400, "Bad Request");

				chfree(filename);
				chfree(u->uri);
				chfree(u);

				return 0;
			}

			uri_params_t *form_params = content_params;

			filedata = NULL;

			while ( form_params ) {

				walker = form_params->next;

				if ( form_params->filename ) {
					if ( filedata ) {
						chfree(filedata);
					}

					filedata = form_params->value;
					file_length = form_params->filesize;

					if (form_params->type ){
						chfree(form_params->type);
					}

					if (form_params->filename) {
						chfree(form_params->filename);
					}

					chfree(form_params);
				} else {
					if (form_params->value ){
						chfree(form_params->value);
					}

					if (form_params->type ){
						chfree(form_params->type);
					}

					if (form_params->filename) {
						chfree(form_params->filename);
					}

					chfree(form_params);
				}

				form_params = walker;
			}
		} else {
			filedata = r->content;
			file_length = r->content_length;
		}
	} else {
		filedata = r->content;
		file_length = r->content_length;
	}

	if ( filedata == NULL ) {
		send_error( 400, "Bad Request");

		chfree(filename);
		chfree(u->uri);
		chfree(u);

		return 0;
	}

	int fd = open(filename, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);

	if ( fd <= 0 ) {
		send_error( 403, "Forbidden");

		chfree(filename);
		chfree(u->uri);
		chfree(u);

		return 0;
	}

	write(fd, filedata, file_length);
	close(fd);

	if ( exists == 0 ) {
		field tempfield;

		tempfield.name = "Location";
		tempfield.value = filename + 5;
		tempfield.next = NULL;

		send_basic_response( 201, "Created", &tempfield);
	} else {
		field tempfield;

		tempfield.name = "Location";
		tempfield.value = filename;
		tempfield.next = NULL;

		send_basic_response( 204, "No Content", &tempfield);
	}

	if ( filename ) {
		chfree(filename);
	}

	if ( u->uri) {
		chfree(u->uri);
	}

	if ( u ) {
		chfree(u);
	}

	encoding *t = NULL;

	while ( encs ) {
		t = encs->next;

		chfree(encs);
		encs = t;
	}

cleanup:
	return 1;
}

int handle_http_v1_delete( request *r )
{
	uri_t *u = NULL;
	int length = 0;
	char *filename = NULL;
	struct stat st;

	if ( r == NULL ) {
		goto cleanup;
	}

	if ( host_field_count( r ) != 1) {
		send_error( 400, "Bad Request");

		return 0;
	}

	u = parse_uri( r->uri );

	if ( u == NULL ) {
		send_error( 500, "Internal Server Error");

		return 1;
	}

	if ( u->uri[0] != '/' ) {
		send_error( 400, "Bad Request");

		chfree(u->uri);
		chfree(u);

		return 0;
	}

	length = 19 + strlen(u->uri);

	filename = challoc( length + 1 );

	if ( filename == NULL ) {
		send_error( 500, "Internal Server Error");

		chfree(u->uri);
		chfree(u);

		return 1;
	}

	memset( filename, 0, length + 1);

	strcpy( filename, "/data");
	strcat( filename, u->uri );

	if ( stat( filename, &st ) ) {
		send_error( 404, "Not Found");

		chfree(filename);
		chfree(u->uri);
		chfree(u);

		return 0;
	}

	if ( (st.st_mode & S_IFREG ) == 0 ) {
		send_error( 403, "Forbidden");

		chfree(filename);
		chfree(u->uri);
		chfree(u);

		return 0;
	}

	int rez = remove(filename);

	if ( rez != 0 ) {
		send_error( 403, "Forbidden");

		chfree(filename);
		chfree(u->uri);
		chfree(u);

		return 0;
	} else {
		send_basic_response(204, "No Content", NULL);
	}

	if ( filename ) {
		chfree(filename);
	}

	if ( u->uri) {
		chfree(u->uri);
	}

	if ( u ) {
		chfree(u);
	}

cleanup:
	return 1;
}

int unknown_method( void )
{
	send_error(405, "Method Not Allowed");

	return 0;
}

int handle_http_v1( request * r )
{
	if ( r == NULL ) {
		return 1;
	}

	switch( r->verb_rt ) {
		case get:
			return handle_http_v1_get(r);
		case post:
			return handle_http_v1_post(r);
		case head:
			return handle_http_v1_head(r);
		case put:
			return handle_http_v1_put(r);
		case delete_e:
			return handle_http_v1_delete(r);
		case connect_e:
			return handle_http_v1_connect(r);
		default:
			return unknown_method( );
	}
	return 0;
}

int handle_http_request( char * data )
{
	if ( data == NULL ) {
		return 1;
	}

	request * r = parse_http_request( data );

	if ( r == NULL ) {
		return 1;
	}

	if ( r->version_major == 1 ) {
		return handle_http_v1( r );
	}

	return 0;
}
