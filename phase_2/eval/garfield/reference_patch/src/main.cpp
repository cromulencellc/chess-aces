#include <iostream>
#include <initializer_list>
#include <vector>
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <fcntl.h>
#include "node.h"
#include "context.hpp"

extern NBlock* programBlock;
extern int yyparse();
extern void yy_switch_to_buffer( struct yy_buffer_state * );
extern struct yy_buffer_state *yy_scan_string( const char * );

FILE *file_out;
FILE *file_in;

Context context_g("global");

// I need a global context containing global variables and functions declared outside of blocks
// This context will need to be a stack of context. When searching for a variable or function it will search from the last thing on the stack first
// When a block is exited the context needs to be popped off of the context stack.

// I need to figure out when to push on a new context and when to remove one.

int sum(const std::initializer_list<int> &il)
{
    int nSum = 0;
    for (auto x: il) 
        nSum += x;
    return nSum;
}

void add_builtin( std::string type, std::string id, const std::initializer_list<std::string> &args )
{
	std::vector<NVariableDeclaration*> varlist;
	const std::string y = "";

	for ( auto x: args ) {
		NIdentifier *a = new NIdentifier(x);
		NIdentifier *b = new NIdentifier("");

		NVariableDeclaration *nvd = new NVariableDeclaration( *a, *b );

		varlist.push_back( nvd );
	}

	NIdentifier *t = new NIdentifier(type);
	NIdentifier *i = new NIdentifier(id);

	NBuiltInFunction *nbf = new NBuiltInFunction( *t, *i, varlist);

	context_g.addBuiltInIdentifier( id, nbf);

	return;
}

void setup_builtins( )
{
	add_builtin( "int", "puts", {"string"});
	add_builtin( "int", "puti", {"int"});
	add_builtin( "double", "putd", {"double"});
	add_builtin( "int", "putl", {"list"});
	add_builtin( "string", "gets", {});
	add_builtin( "void", "exit", {"int"});
	add_builtin( "int", "len", {"*"});
	add_builtin( "int", "atoi", {"string"});
	add_builtin( "string", "substr", {"string", "int", "int"});
	add_builtin( "void", "setchr", {"string", "string", "int"} );
	add_builtin( "int", "strstr", {"string", "string"});
	add_builtin( "list", "strtok", {"string", "string"});
	add_builtin( "string", "itos", {"int"});
	add_builtin( "string", "dtos", {"double"});
	add_builtin( "string", "type", {"*"});
	add_builtin( "int", "append", {"list", "*"});
	add_builtin( "int", "prepend", {"list", "*"});
	add_builtin( "*", "popend", {"list" });
	add_builtin( "*", "popfront", {"list" });
	add_builtin( "*", "getbyindex", {"list", "int"});
	add_builtin( "int", "exists", {"list", "*"});
	add_builtin( "void", "erase", {"list", "int"});
	add_builtin( "string", "str", {"*"});
	add_builtin( "void", "tolower", {"string"});
	add_builtin( "void", "toupper", {"string"});
	add_builtin( "string", "hex", {"int"});
	add_builtin( "string", "bin", {"int"});
	add_builtin( "string", "oct", {"int"});
	add_builtin( "int", "sum", {"list"});
	add_builtin( "int", "max", {"int", "int"});
	add_builtin( "int", "min", {"int", "int"});
}

void scan_string(const char* str)
{
    yy_switch_to_buffer(yy_scan_string(str));
}

#define MAXREAD 1024
char *readuntil( int fd, char c )
{
	char *line = (char *)calloc( 1, MAXREAD + 1);
	char t = 0;
	int index = 0;

	if ( !line ) {
		return NULL;
	}

	while ( read( fd, &t, 1 ) > 0 && index < MAXREAD ) {
		line[index] = t;
		index++;

		if ( t == c ) {
			return line;
		}
	}

	if ( index == 0 ) {
		free(line);
		line = NULL;
	}

	return NULL;
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

int main(int argc, char **argv)
{
	int server_flag = 0;
	int file_flag = 0;
	int connfd, fd = 0;
	int PORT = 3004;
	char c;
	char *fname;

	int infd = fileno(stdin);
	int outfd = fileno(stdout);

	if ( getenv("PORT") ) {
		PORT = atoi(getenv("PORT") );
	}

    char *b = getenv("CHESS");
    if ( b == NULL ) {
        std::cout << "[TESTBED] ENV variable check failed" << std::endl;
        exit(0);
    }

	while ((c = getopt (argc, argv, "p:sf:")) != -1) {
		switch (c) {
			case 'p':
				PORT = atoi( optarg );
				break;
			case 's':
				server_flag = 1;
				break;
			case 'f':
				file_flag = 1;
				fname = strdup( optarg );
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

	setup_builtins();

	if ( server_flag ) {
		fd = setup_socket( PORT );

		connfd = accept_conn( fd );

		if ( connfd < 0 ) {
			exit(0);
		}

		infd = connfd;
		outfd = connfd;
	} else if (file_flag ) {
		infd = open(fname, O_RDONLY);
	}

	file_out = fdopen(dup(outfd), "w");
	file_in = fdopen(dup(infd), "r");

	fprintf(file_out, ">>> ");
	fflush(file_out);
	
	char *line = readuntil( infd, '\n' );

	Value *last = NULL;

	while ( line ) {
		programBlock = NULL;

		scan_string( line );

		yyparse();

		free(line);

		if ( programBlock != NULL ) {
		    for ( auto it = programBlock->statements.begin(); it != programBlock->statements.end(); ++it) {
		    	try {
		    		if ( last != NULL ) {
		    			delete last;
		    			last = NULL;
		    		}

		    		// TODO: print last return value

		    		last = (**it).execute(NULL);
		    	} catch (InvalidOperationException& e) {
		    		fprintf(file_out, "%s: %s\n", e.what(), e.id.c_str());
		    		break;
		    	} catch ( InvalidTypeException& e ) {
		    		fprintf(file_out, "%s: %s\n", e.what(), e.id.c_str());
		    		break;
		    	} catch ( GenericException& e ) {
		    		fprintf(file_out, "%s: %s\n", e.what(), e.id.c_str());
		    		break;
		    	} catch ( InvalidIdentifierException& e ) {
		    		fprintf(file_out, "%s: %s\n", e.what(), e.id.c_str());
		    		break;
		    	} catch ( UnknownFunctionException& e ) {
		    		fprintf(file_out, "%s: %s\n", e.what(), e.id.c_str());
		    		break;
		    	} catch ( InvalidFunctionArgCountException& e ) {
		    		fprintf(file_out, "%s: %s\n", e.what(), e.id.c_str());
		    		break;
		    	} catch ( DivByZeroException& e ) {
		    		fprintf(file_out, "%s: %s\n", e.what(), e.id.c_str());
		    		break;
		    	}
		    }
		}

		fprintf(file_out, ">>> ");
		fflush(file_out);

		line = readuntil( infd, '\n' );

	}

    return 0;
}
