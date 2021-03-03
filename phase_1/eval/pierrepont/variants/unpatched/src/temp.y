%{

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "matrix.h"
#include "matrix_vars.h"
#include "matrix_funcs.h"
#include "testbed.h"

extern int yylex();
extern int yyparse();
extern FILE* yyin;
FILE *file_out = NULL;

int PORT = 3004;

void yyerror(const char* s);

extern void scan_string( const char *);

%}

%union {
	char *var;
	char *func;
	struct matrix *m;
}

%token <var> NAME
%token PLUS MINUS MULTIPLY LPAREN RPAREN ASSIGN SINGLE_QUOTE
%token NEWLINE QUIT COMMA HAT DOT_HAT
%token DOT_DIVIDE DOT_MULTIPLY BARS
%token<m> MATRIX
%token<func> FUNCTION
%left PLUS MINUS
%left MULTIPLY DIVIDE
%left ASSIGN

%type<m> matrix_math

%start calculation

%%

calculation:
	   | calculation line
;

line: NEWLINE
    | matrix_math NEWLINE {
    	if ( $1 != NULL ) {
	    	print_matrix( $1 );

	    	add_var( "ans", $1);
    	} 
    }
    | QUIT NEWLINE { fprintf(file_out, "bye!\n"); exit(0); }
    | NAME ASSIGN matrix_math NEWLINE {
    	if ( $3 != NULL ) {
    		add_var( $1, $3); 
    		fprintf(file_out, "%s = \n", $1); 
    		print_matrix($3);
    	}
    }
;

matrix_math: MATRIX 	{ $$ = $1; }
	| matrix_math PLUS matrix_math { $$ = add_matrices( $1, $3); }
	| matrix_math HAT matrix_math { $$ = exp_matrix( $1, $3); }
	| matrix_math DOT_HAT matrix_math { $$ = exp_matrix_elw( $1, $3); }
	| matrix_math MINUS matrix_math { $$ = sub_matrices( $1, $3); }
	| matrix_math DOT_MULTIPLY matrix_math { $$ = dot_multiply_matrices ( $1, $3); }
	| matrix_math DOT_DIVIDE matrix_math { $$ = dot_divide_matrices ( $1, $3); }
	| matrix_math MULTIPLY matrix_math { $$ = multiply_matrices ( $1, $3); }
	| matrix_math SINGLE_QUOTE { $$ = matrix_transpose( $1 ); }
	| FUNCTION LPAREN matrix_math RPAREN { $$ = handle_func_single_arg( $1, $3); }
	| FUNCTION LPAREN matrix_math COMMA matrix_math RPAREN { $$ = handle_func_two_arg( $1, $3, $5); }
	| BARS matrix_math BARS { $$ = magnitude( $2 ); }
	| LPAREN matrix_math RPAREN		{ $$ = $2; }
	| NAME {
    	matrix *tempm = get_matrix_copy( $1 );

    	if ( tempm != NULL ) {
    		$$ = tempm;
    	} else {
    		fprintf(file_out, "error %s -- unknown variable\n", $1);
    		$$ = NULL;
    	}
    }
;

%%

#define MAXREAD 1024
char *readuntil( int fd, char c )
{
	char *line = calloc( 1, MAXREAD + 1);
	char t;
	int index = 0;

	if ( !line ) {
		return NULL;
	}

	while ( read( fd, &t, 1 ) >= 0 && index < MAXREAD ) {
		line[index] = t;
		index++;

		if ( t == c ) {
			return line;
		}
	}

	return line;
}

char *readline( FILE *n )
{
	char *lineptr = NULL;
	size_t length = 0;

	if ( !n ) {
		return NULL;
	}

	getline(&lineptr, &length, n);

	return lineptr;
}

int setup_socket( int port )
{
	int fd;
	struct sockaddr_in sa;
	int enable = 1;

	fd = socket( AF_INET, SOCK_STREAM, 0);

	if ( fd < 0 ) {
		fprintf(stderr, "socket fail\n");
		exit(0);
	}

	memset( &sa, 0, sizeof(struct sockaddr_in));

	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(port);

	// Bind the socket to the port
	if ( bind( fd, (struct sockaddr *)&sa, sizeof(sa) ) < 0 ) {
		fprintf(stderr, "Error on binding\n");
		close(fd);
		return -1;
	}

	if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(enable)) < 0 ) {
		fprintf(stderr, "Error on setsockopt\n");
		close(fd);
		return -1;
	}

	if ( listen(fd, 0 ) < 0 ) {
		fprintf(stderr, "Error on listen\n");
		close(fd);
		return -1;
	}

	fprintf(stdout, "[INFO] Listener socket on port: %d\n", port);

	return fd;
}

int accept_conn( int fd )
{
	int conn_fd = 0;

	struct sockaddr_in ca;
	socklen_t ca_len = sizeof( struct sockaddr_in );

	memset(&ca, 0, sizeof(struct sockaddr_in));

	conn_fd = accept( fd, (struct sockaddr *)&ca, &ca_len);

	if ( conn_fd < 0 ) {
		fprintf(stderr, "Error accepting client.\n");
		close(fd);
		return -1;
	}

	/// I no longer need the server
	close(fd);

	return conn_fd;
}

void usage( char *pn )
{
	fprintf(stderr, "USAGE: %s -p <port> -s\n", pn);
	fprintf(stderr, "The -s option is used to specify a client/server interaction\n");
	fprintf(stderr, "The -p option is optional and defaults to 3004. It can also be specified via the PORT environment variable.\n");

	exit(1);
	return;
}

int main(int argc, char **argv, char *envp[])
{
	int server_flag = 0;
	int connfd, fd = 0;
	char c;

	#ifndef NO_TESTBED
        assert_execution_on_testbed();
        #endif

	int infd = fileno(stdin);
	int outfd = fileno(stdout);

	if ( getenv("PORT") ) {
		PORT = atoi(getenv("PORT") );
	}

	while ((c = getopt (argc, argv, "p:s")) != -1) {
		switch (c) {
			case 'p':
				PORT = atoi( optarg );
				break;
			case 's':
				server_flag = 1;
				break;
			case '?':
        		if (optopt == 'p') {
          			fprintf (stderr, "-%c argument required\n", optopt);
          			usage(argv[0]);
      			} else {
        			fprintf (stderr, "Unknown option\n");
        			usage(argv[0]);
        		}
      		default:
        		exit(1);
		}
	}

	if ( server_flag ) {
		fd = setup_socket( PORT );

		connfd = accept_conn( fd );

		if ( connfd < 0 ) {
			exit(0);
		}

		printf("Received connecting\n");

		infd = connfd;
		outfd = connfd;
	}

	file_out = fdopen(dup(outfd), "w");

	fprintf(file_out, ">>> ");
	fflush(file_out);
	
	char *line = readuntil( infd, '\n' );

	while ( line ) {
		scan_string( line );

		yyparse();

		free(line);

		fprintf(file_out, ">>> ");
		fflush(file_out);

		line = readuntil( infd, '\n' );
	}

	return 0;
}

void yyerror(const char* s) {
	fprintf(file_out, "Parse error: %s\n", s);
}
