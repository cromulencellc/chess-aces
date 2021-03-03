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
#include <utmp.h>
#include <time.h>
#include "testbed.h"

typedef struct connection_info
{
	int fd;
	int helo_done;
	int mail_from_done;
	int authd;
	char authd_user[32];
	char *clnt_hn;
	char temp_data[256];
	char *clnt_ip;
	char *srv_hn;
	char *mail_from;
	char **rcpt_to;
	int rcpt_to_cnt;
	int rcpt_to_max;
	char * data;
	int data_max_len;
	int data_current_len;

	
	
	
	
	int type;
} coninfo_t, *pconinfo_t;

typedef struct email
{
	char name[64];
	char domain[64];
	struct email *left;
	struct email *right;
} email_t, *pemail_t;

typedef struct domainlist
{
	char name[64];
	pemail_t email_root;
	struct domainlist *next;
} domainlist_t, *pdomainlist_t;

char *emaillist = "/base_data/emaillist.txt";
char *userlist = "/base_data/users.txt";
char *maillist = "/base_data/mailinglist.txt";
int port;

coninfo_t client_data;
domainlist_t *domain_root;

char gline[128];
char *yolo[] = {".mbx", "From: ", "To: "};

int local_strlen( const char *src )
{
	int i = 0;

	if ( !src ) {
		return 0;
	}

	while ( src[i] != '\x00') {
		i++;
	}

	return i;

}

char *local_strstr( const char *src, const char *sub)
{
	int index = 0;
	int len_sub = 0;
	int len_src = 0;

	if ( !src || !sub ) {
		return NULL;
	}

	len_sub = local_strlen(sub);

	len_src = local_strlen(src);

	if ( len_src < len_sub ) {
		return NULL;
	}

	while ( index < (len_src - len_sub) + 1 ) {
		if ( src[index] == sub[0] ) {
			if ( memcmp( src + index, sub, len_sub) == 0 ) {
				return (char *)(src + index);
			}
		}
		
		index += 1;
	}

	return NULL;
}

char *local_strcat( char *dest, const char *src )
{
	int i = 0;
	int j = 0;
	int l;

	if ( !dest ||  !src ) {
		return dest;
	}

	i = local_strlen(dest);
	l = local_strlen(src);

	for( j = 0; j < l; i++, j++) {
		dest[i] = src[j];
	}

	return dest;
}

char *get_date_time ( )
{
	char *time_str = NULL;
	struct tm *ct = NULL;
	time_t rawtime;
	char td[32];

	time_str = malloc( 256 );

	if ( !time_str ) {
		return time_str;
	}

	memset( time_str, 0, 256 );
	memset( &rawtime, 0, sizeof(rawtime) );
	memset( td, 0, 32);

	time( &rawtime);
	ct = localtime( &rawtime );

	if ( ct == NULL ) {
		return NULL;
	}

	switch ( ct->tm_wday ) {
		case 0:
			local_strcat( time_str, "Sun ");
			break;
		case 1:
			local_strcat( time_str, "Mon ");
			break;
		case 2:
			local_strcat( time_str, "Tue ");
			break;
		case 3:
			local_strcat( time_str, "Wed ");
			break;
		case 4:
			local_strcat( time_str, "Thu ");
			break;
		case 5:
			local_strcat( time_str, "Fri ");
			break;
		case 6:
			local_strcat( time_str, "Sat ");
			break;
		default:
			local_strcat( time_str, "Unk ");
			break;
	}

	switch ( ct->tm_mon ) {
		case 0:
			local_strcat( time_str, "Jan ");
			break;
		case 1:
			local_strcat( time_str, "Feb ");
			break;
		case 2:
			local_strcat( time_str, "Mar ");
			break;
		case 3:
			local_strcat( time_str, "Apr ");
			break;
		case 4:
			local_strcat( time_str, "May ");
			break;
		case 5:
			local_strcat( time_str, "Jun ");
			break;
		case 6:
			local_strcat( time_str, "Jul ");
			break;
		case 7:
			local_strcat( time_str, "Aug ");
			break;
		case 8:
			local_strcat( time_str, "Sep ");
			break;
		case 9:
			local_strcat( time_str, "Oct ");
			break;
		case 10:
			local_strcat( time_str, "Nov ");
			break;
		case 11:
			local_strcat( time_str, "Dec ");
			break;
		default:
			local_strcat( time_str, "Unk ");
			break;
	}

	snprintf(td, 32, "%d %.2d:%.2d:%.2d %d\n", ct->tm_mday, ct->tm_hour, ct->tm_min, ct->tm_sec, ct->tm_year + 1900);

	local_strcat( time_str, td);

	return time_str;
}

int write_wrapper( int fd, char *buffer, int length ) 
{
	int result;

	if ( buffer == NULL ) {
		return 0;
	}

	if ( length <= 0 ) {
		return 0;
	}

	result = write( fd, buffer, length );

	if ( result < 0 ) {
		return 0;
	}

	return 1;
}

int send_string( int fd, char *str)
{
	int len;

	if (str == NULL ) {
		return 0;
	}

	len = local_strlen(str);

	write_wrapper( fd, str, len );

	return len;
}

void print_domains( )
{
	pdomainlist_t walker = domain_root;

	while ( walker ) {
		printf("Domain: %s\n", walker->name);
		if ( walker->email_root == NULL ) {
		}
		walker = walker->next;
	}

	return;
}

int parse_domain_name( char *dn )
{
	int length = 0;
	int index = 0;
	int last_char_dot = 0;

	if ( dn == NULL ) {
		return 0;
	}

	length = local_strlen(dn);

	while ( index < length ) {
		if ( isalnum( dn[index] ) ) {
			index++;
			last_char_dot = 0;
		} else if ( dn[index] == '.' && last_char_dot == 0) {
			last_char_dot = 1;
			index++;
		} else {
			return 0;	
		}
	}

	return 1;
}

int readuntil( int fd, char *buffer, int maxlen, char terminator)
{
	char c;
	int count = 0;

	if ( buffer == NULL ) {
		return 0;
	}

	do {
		if ( read( fd, &c, 1 ) <= 0 ) {
			return 0;
		}

		if ( c == terminator) {
		       return count;
		}

		buffer[count] = c;
		count++;		
	} while ( count < maxlen);

	
	buffer[count] = '\x00';
	return count;
}

int domain_exists( char *domain )
{
	pdomainlist_t walker = NULL;

	if ( domain == NULL ) {
		return 1;
	}

	if ( domain_root == NULL ) {
		return 0;
	}

	walker = domain_root;

	while ( walker ) {
		if ( strcmp( domain, walker->name) == 0 ) {
			return 1;
		}

		walker = walker->next;
	}

	return 0;
}

int init_domain( char *domain )
{
	pdomainlist_t nd = NULL;

	if ( domain == NULL ) {
		return -1;
	}

	if ( domain_exists( domain) ) {
		return -1;
	}

	nd = malloc( sizeof(domainlist_t ) );

	if ( nd == NULL ) {
		printf("[ERROR] Allocation failure\n");
		return -1;
	}

	memset( nd, 0, sizeof( domainlist_t) );

	if ( domain_root == NULL ) {
		domain_root = nd;
	} else {
		nd->next = domain_root->next;
		domain_root->next = nd;
	}

	
	strncpy(nd->name, domain, 64);

	return 0;
}

pdomainlist_t getdomain ( char *domain )
{
	pdomainlist_t walker = NULL;

	if ( domain == NULL ) {
		return NULL;
	}

	walker = domain_root;

	while ( walker ) {
		if (strcmp( walker->name, domain ) == 0 ) {
			return walker;
		}

		walker = walker->next;
	}

	return NULL;
}

void print_tree( pemail_t root )
{
	if ( root == NULL ) {
		return;
	}

	if ( root->left ) {
		print_tree( root->left );
	} 
	printf("%s@%s\n", root->name, root->domain);

	if ( root->right ) {
		print_tree( root->right );
	}

	return;
}



int add_email_leaf( pdomainlist_t dm, char *email)
{
	pemail_t pm = NULL;
	pemail_t walker = NULL;
	int result = 0;

	if ( dm == NULL || email == NULL) {
		return -1;
	}

	pm = malloc( sizeof(email_t) );

	if ( pm == NULL ) {
		return -1;
	}

	memset(pm, 0, sizeof(email_t));
	strncpy( pm->name, email, 64 );
	strncpy( pm->domain, dm->name, 64 );

	if ( dm->email_root == NULL ) {
		dm->email_root = pm;
		return 0;
	}

	walker = dm->email_root;
	
	while ( 1 ) {
		result = strcmp( walker->name, email);

		if ( result == 0 ) {
			free( pm );
			return 0;
		} else if ( result > 0 ) {
			if ( walker->left == NULL ) {
				walker->left = pm;
				return 0;
			} else {
				walker = walker->left;
			}
		} else {
			if ( walker->right == NULL ) {
				walker->right = pm;
				return 0;
			} else {
				walker = walker->right;
			}
		}
	}

	return 0;
}

int add_email( char *email, char *domain )
{
	pdomainlist_t dm = NULL;

	if ( email == NULL || domain == NULL ) {
		return -1;
	}

	dm = getdomain ( domain );

	if ( dm == NULL ) {
		printf("[ERROR] Failed to locate\n");
		return -1;
	}

	add_email_leaf( dm, email );


	return 0;
}

int ingest_emaillist(  )
{
	int fd = 0;
	char line[256];
	char nm[64];
	char domain[64];
	char *dp = NULL;

	fd = open(emaillist, O_RDONLY );

	if ( fd <= 0 ) {
		printf("[ERROR] Failed to open list: %s\n", emaillist);
		return 0;
	}

	memset( line, 0, 256 );

	while ( readuntil(fd, line, 256, '\n') ) {
		memset(nm, 0, 64);
		memset( domain, 0, 64 );

		dp = local_strstr( line, "@");

		if ( dp == NULL ) {
			printf("[ERROR] Improper formatting\n");
			continue;
		}

		dp[0] = '\x00';
		dp++;

		strncpy( nm, line, 64 );
		strncpy( domain, dp, 64 );

		init_domain( domain );

		add_email( nm, domain );

		memset(line, 0, 256);
	}

	close(fd);

	return 1;
}


int setup_socket( int port )
{
	int fd;
	struct sockaddr_in sa;
	int enable = 1;

	fd = socket( AF_INET, SOCK_STREAM, 0);

	if ( fd < 0 ) {
		printf("[ERROR] 1\n");
		exit(0);
	}

	memset( &sa, 0, sizeof(struct sockaddr_in));

	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(port);

	
	if ( bind( fd, (struct sockaddr *)&sa, sizeof(sa) ) < 0 ) {
		printf("Error on binding\n");
		close(fd);
		return -1;
	}

	if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0 ) {
		printf("Error on setsockopt\n");
		close(fd);
		return -1;
	}

	if ( listen(fd, 0 ) < 0 ) {
		printf("Error on listen\n");
		close(fd);
		return -1;
	}

	printf("[INFO] Listener socket on port: %d\n", port);

	return fd;
}

int valid_email( char *email )
{
	char nm[64];
	char *temp_domain = NULL;
	pdomainlist_t pdml = NULL;
	pemail_t walker = NULL;
	int result = 0;

	if ( email == NULL ) {
		return 0;
	}

	memset(nm, 0, 64);

	temp_domain = local_strstr( email, "@" );

	if ( temp_domain == NULL ) {
		return 0;
	}

	if ( temp_domain == email ) {
		return 0;
	}

	pdml = getdomain ( temp_domain + 1 );

	if ( pdml == NULL ) {
		return 0;
	}

	
	if ( temp_domain - email < 64 ) {
		memcpy( nm, email, temp_domain-email );
	} else {
		return 0;
	}

	walker = pdml->email_root;

	while ( 1 ) {
		result = strcmp( walker->name, nm );

		if ( result == 0 ) {
			return 1;
		} else if ( result > 0 ) {
			if ( walker->left == NULL ) {
				return 0;
			} else {
				walker = walker->left;
				continue;
			}
		} else {
			if ( walker->right == NULL ) {
				return 0;
			} else {
				walker = walker->right;
				continue;
			}
		}
	}
	
	return 0;
}

void cleanup_global ( )
{
	int i = 0;

	if ( client_data.clnt_ip != NULL ) {
		free(client_data.clnt_ip);
		client_data.clnt_ip = NULL;
	}

	if ( client_data.clnt_hn != NULL ) {
		free( client_data.clnt_hn );
		client_data.clnt_hn = NULL;
	}

	if ( client_data.srv_hn != NULL ) {
		free(client_data.srv_hn ) ;
		client_data.srv_hn = NULL;
	}

	if ( client_data.mail_from != NULL ) {
		free( client_data.mail_from );
		client_data.mail_from = NULL;
	}

	if ( client_data.rcpt_to != NULL ) {
		for (i = 0; i < client_data.rcpt_to_cnt; i++ ) {
			if ( client_data.rcpt_to[i] != NULL ) {
				free(client_data.rcpt_to[i]);
				client_data.rcpt_to[i] = NULL;
			}
		}

		free( client_data.rcpt_to );
		client_data.rcpt_to_cnt = 0;
		client_data.rcpt_to_max = 0;
	}

	if ( client_data.data != NULL ) {
		free(client_data.data);
		client_data.data = NULL;
	}

	client_data.data_max_len = 0;
	client_data.data_current_len = 0;

	client_data.helo_done = 0;
	client_data.mail_from_done = 0;
	client_data.authd = 0;
	client_data.type = 0;

	return;
}

int search_email_substr( char *em, pdomainlist_t pdml )
{
	pemail_t walker = NULL;
	int result = 0;

	if ( em == NULL || pdml == NULL ) {
		return 0;
	}

	walker = pdml->email_root;

	if ( walker == NULL ) {
		printf("Something is broken: %s\n", pdml->name);
	}

	while ( 1 ) {
		result = strcmp( walker->name, em );

		if ( result == 0 ) {
			return 1;
		} else if ( result > 0 ) {
			if ( walker->left == NULL ) {
				return 0;
			} else {
				walker = walker->left;
				continue;
			}
		} else {
			if ( walker->right == NULL ) {
				return 0;
			} else {
				walker = walker->right;
				continue;
			}
		}
	}

	return 0;
}

int search_all_domains( char *em )
{
	pdomainlist_t walker = NULL;

	if ( em == NULL ) {
		return 0;
	}

	walker = domain_root;

	while ( walker ) {

		if ( search_email_substr( em, walker ) ) {
			return 1;
		}

		walker = walker->next;
	}

	return 0;
}

int handle_VRFY( int fd, char *line )
{
	char *temp_domain = NULL;
	char response[256];

	if ( line == NULL ) {
		return 0;
	}

	memset( response, 0, 256);

	temp_domain = local_strstr( line, "@" );

	if ( temp_domain != NULL ) {
		if ( valid_email( line ) ) {
			snprintf(response, 256, "250 %s\n", line);
			write_wrapper(fd, response, local_strlen(response));
		} else {
			send_string(fd, "550 Doesn't match\n");
		}
	} else {
		if ( search_all_domains( line ) ) {
			snprintf(response, 256, "250 %s\n", line);
			write_wrapper(fd, response, local_strlen(response) );
		} else {
			snprintf(response, 256, "550 Doesn't match\n");
			write_wrapper(fd, response, local_strlen(response) );
		}

	}
	return 0;
}

int handle_HELO( int fd, char *line, struct sockaddr_in *sin) 
{
	char ip_addr[32];
	char *response = NULL;
	char hostname[256];

	if ( line == NULL || sin == NULL) {
		return 0;
	}

	memset(ip_addr, 0, 32);
	memset(hostname, 0, 256);

	if ( inet_ntop( AF_INET, &sin->sin_addr, ip_addr, 32) == NULL ) {
		printf("[ERROR] Failed to convert ip\n");
		return 0;
	}

	response = malloc( 512 );

	if ( response == NULL ) {
		printf("[ERROR] failed to allocate response\n");
		return 0;
	}

	memset( response, 0, 512);

	if ( parse_domain_name(line) == 0 ) {
		snprintf(response, 512, "%d Domain name fail\n", 501);

		write_wrapper( fd, response, local_strlen(response) );
		free(response);

		return 0;
	}

	gethostname( hostname, 256 );

	if ( client_data.clnt_hn == NULL ) {
		client_data.clnt_hn = malloc( 32 );

		if ( client_data.clnt_hn == NULL ) {
			free(response);
			return 0;
		}

		memset( client_data.clnt_hn, 0, 32 );
	}

	if ( client_data.clnt_ip == NULL ) {
		client_data.clnt_ip = malloc( 32 );

		if ( client_data.clnt_ip == NULL ) {
			free(response);
			return 0;
		}

		memset( client_data.clnt_ip, 0, 32 );
	}

	if ( client_data.srv_hn == NULL ) {
		client_data.srv_hn = malloc( 256 );

		if ( client_data.srv_hn == NULL ) {
			free(response);
			return 0;
		}

		memset( client_data.srv_hn, 0, 256);
	}

	strncpy( client_data.clnt_ip, ip_addr, 31 );
	strncpy( client_data.srv_hn, hostname, 32 );
	strncpy( client_data.clnt_hn, line, 31 );

	snprintf(response, 512, "%d %s Sup %s [%s] Welcome\n", 250, client_data.srv_hn, client_data.clnt_hn, client_data.clnt_ip);

	write_wrapper(fd, response, local_strlen(response));

	free(response);

	client_data.helo_done = 1;

	return 1;
}

int handle_QUIT( int fd )
{
	char response[256];

	memset( response, 0, 256 );

	if ( client_data.srv_hn == NULL ) {
		client_data.srv_hn = malloc(256);

		if ( client_data.srv_hn == NULL ) {
			send_string(fd, "562 Not ok\n");
			return 1;
		} else {
			gethostname( client_data.srv_hn, 256 );
		}
	}

	snprintf(response, 256, "221 %s au revoir\n", client_data.srv_hn );
	write_wrapper(fd, response, local_strlen(response) );

	return 0;
}

int is_valid_email( const char *email)
{
	char *at_spot = NULL;
	char *dot_spot = NULL;
	char temp_email[256];
	int i;

	if ( email == NULL ) {
		return 0;
	}

	memset( temp_email, 0, 256 );
	strncpy( temp_email, email, 256);

	at_spot = local_strstr(temp_email, "@");

	if ( at_spot == NULL ) {
		return 0;
	}

	
	if ( at_spot == temp_email) {
		return 0;
	}

	at_spot[0] = '\x00';

	if ( at_spot - temp_email > 64 ) {
		return 0;
	}

	for ( i = 0; i < at_spot-temp_email; i++ ) {
		if ( !isalnum( temp_email[i] ) ) {
			return 0;
		}
	}

	at_spot += 1;

	dot_spot = local_strstr( at_spot, ".");

	if ( dot_spot == NULL ) {
		return 0;
	}

	if ( dot_spot - at_spot > 60 ) {
		return 0;
	}

	for ( i = 0; i < dot_spot - at_spot; i++ ) {
		if ( !isalnum( at_spot[i]) && at_spot[i] != '-' ) {
			return 0;
		}
	}

	dot_spot += 1;

	if ( local_strlen( dot_spot) != 3 ) {
		return 0;
	}

	for ( i = 0; i < 3; i++) {
		if ( !isalpha( dot_spot[i] ) ) {
			return 0;
		}
	}


	return 1;
}

int handle_MAIL_FROM( int fd, char *line )
{
	char response[256];
	char *end = NULL;
	char email[256];

	if ( line == NULL ) {
		return 0;
	}

	memset(response, 0, 256 );
	memset( email, 0, 256 );

	if ( client_data.helo_done != 1 ) {
		snprintf(response, 256, "503 A little too soon\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	if ( line[0] != ':' ) {
		snprintf(response, 256, "510 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	line++;

	if ( line[0] != '<' ) {
		snprintf(response, 256, "511 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	line++;

	end = local_strstr( line, ">");

	if ( end == NULL ) {
		snprintf(response, 256, "512 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	if ( end - line < 256 ) {
		memcpy( email, line, end-line );
	}

	if ( !is_valid_email(email) ) {
		snprintf(response, 256, "513 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	client_data.mail_from = malloc( local_strlen(email) + 1 );

	if ( client_data.mail_from == NULL ) {
		return 0;
	}

	memset( client_data.mail_from, 0, local_strlen( email ) + 1 );

	strcpy( client_data.mail_from, email );

	

	client_data.mail_from_done = 1;

	snprintf(response, 256, "250 OK\n");
	write_wrapper( fd, response, local_strlen(response) );

	client_data.type = 1;

	return 0;
}

int get_username( const char *email, char *user )
{
	char *at_spot = NULL;

	if ( email == NULL || user == NULL) {
		return 0;
	}

	at_spot = local_strstr(email, "@");

	if ( at_spot == NULL ) {
		return 0;
	}

	if ( at_spot - email > 64 ) {
		return 0;
	}

	strncpy( user, email, at_spot - email );

	

	return 1;
}

int is_writeable( char *tty )
{
	struct stat st;
	char path[ 512 ];

	if ( tty == NULL ) {
		return 0;
	}

	memset( &st, 0, sizeof( struct stat ) );
	memset( path, 0, 512);

	snprintf(path, 512, "%s%s", _PATH_DEV, tty);

	if ( stat(path, &st) != 0 ) {
		return 0;
	}

	return st.st_mode & (S_IWRITE >> 3) ;
}

int user_logged_in( char *user )
{
	int fd = 0;
	struct utmp ut;

	if ( user == NULL ) {
		return 0;
	}

	memset( &ut, 0, sizeof( ut ) );

	fd = open( _PATH_UTMP, O_RDONLY );

	if ( fd <= 0 ) {
		return 0;
	}

	while ( read(fd, (char *)&ut, sizeof(ut)) == sizeof(ut)) {
		if ( strcmp( user, ut.ut_name ) == 0 ) {
			close(fd);
			return 1;
		}
	}

	close(fd);

	return 0;
}


int find_tty( const char *user, char *tty )
{
    int fd;
    struct utmp ut;

    if ( user == NULL || tty == NULL ) {
            return 0;
    }

    memset( &ut, 0, sizeof( ut ) );

    fd = open( _PATH_UTMP, O_RDONLY );

    if ( fd <= 0 ) {
        return 0;
    }

    while ( read(fd, (char *)&ut, sizeof(ut)) == sizeof(ut)) {
        if ( strcmp( user, ut.ut_name ) == 0 ) {
            if ( is_writeable( ut.ut_line ) ) {
                strncpy( tty, ut.ut_line, 256 );

                close(fd);
                return 1;
            }
        }
    }

    close(fd);

    return 0;
}

int handle_RCPT_TO( int fd, char *line )
{
	char response[256];
	char *end = NULL;
	char email[256];
	char **temp_array = NULL;
	char un[256];

	if ( line == NULL ) {
		return 0;
	}

	memset(response, 0, 256 );
	memset( email, 0, 256 );
	memset(un, 0, 256 );

	if ( !client_data.helo_done  || !client_data.mail_from_done ) {
		snprintf(response, 256, "503 A little too soon\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	if ( line[0] != ':' ) {
		snprintf(response, 256, "510 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	line++;

	if ( line[0] != '<' ) {
		snprintf(response, 256, "511 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	line++;

	end = local_strstr( line, ">");

	if ( end == NULL ) {
		snprintf(response, 256, "511 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	if ( end - line < 256 ) {
		memcpy( email, line, end-line );
	}

	if ( !is_valid_email(email) ) {
		snprintf(response, 256, "511 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	if ( !valid_email(email) ) {
		snprintf(response, 256, "512 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	
	if ( client_data.type == 2 || client_data.type == 4) {
		if ( !get_username( email, un ) ) {
			snprintf(response, 256, "511 Invalid address\n");
			write_wrapper( fd, response, local_strlen(response) );
			return 0;
		}

		if ( !user_logged_in( un ) ) {
			snprintf(response, 256, "450 Not logged in\n");
			write_wrapper( fd, response, local_strlen(response) );
			return 0;
		}
	}

	
	if ( client_data.rcpt_to == NULL ) {
		client_data.rcpt_to = malloc(sizeof(char *) * 10);

		if ( client_data.rcpt_to == NULL ) {
			printf("Error allocating\n");
			return 0;
		}

		client_data.rcpt_to_cnt = 0;
		client_data.rcpt_to_max = 10;
	}

	
	if ( client_data.rcpt_to_cnt == client_data.rcpt_to_max ) {

		if ( client_data.rcpt_to_max == 20 ) {
			snprintf(response, 256, "547 Max rcpt reached\n");
			write_wrapper( fd, response, local_strlen(response) );
			return 0;
		}

		temp_array = malloc(sizeof(char *) * 20);

		if ( temp_array == NULL ) {
			printf("Error allocating\n");
			return 0;
		}

		memcpy( temp_array, client_data.rcpt_to, sizeof(char*) * 10);

		free( client_data.rcpt_to);
		client_data.rcpt_to = temp_array;
		client_data.rcpt_to_max = 20;
	}

	client_data.rcpt_to[ client_data.rcpt_to_cnt ]  = malloc( local_strlen(email) + 1 );

	if ( client_data.mail_from == NULL ) {
		return 0;
	}

	memset( client_data.rcpt_to[ client_data.rcpt_to_cnt ], 0, local_strlen( email ) + 1 );

	strcpy( client_data.rcpt_to[ client_data.rcpt_to_cnt ], email );

	snprintf(response, 256, "250 OK\n");
	write_wrapper( fd, response, local_strlen(response) );

	client_data.rcpt_to_cnt++;

	return 1;
}

int handle_RSET( int fd )
{
	int i = 0;
	char response[256];

	memset( response, 0, 256 );

	if ( client_data.clnt_ip != NULL ) {
		free(client_data.clnt_ip);
		client_data.clnt_ip = NULL;
	}

	if ( client_data.clnt_hn != NULL ) {
		free( client_data.clnt_hn );
		client_data.clnt_hn = NULL;
	}

	if ( client_data.srv_hn != NULL ) {
		free(client_data.srv_hn ) ;
		client_data.srv_hn = NULL;
	}

	if ( client_data.mail_from != NULL ) {
		free( client_data.mail_from );
		client_data.mail_from = NULL;
	}

	if ( client_data.rcpt_to != NULL ) {
		for (i = 0; i < client_data.rcpt_to_cnt; i++ ) {
			if ( client_data.rcpt_to[i] != NULL ) {
				free(client_data.rcpt_to[i]);
				client_data.rcpt_to[i] = NULL;
			}
		}

		free( client_data.rcpt_to );
		client_data.rcpt_to_cnt = 0;
		client_data.rcpt_to_max = 0;
	}

	if ( client_data.data != NULL ) {
		free(client_data.data);
		client_data.data = NULL;
	}

	client_data.data_max_len = 0;
	client_data.data_current_len = 0;

	client_data.helo_done = 0;
	client_data.mail_from_done = 0;
	client_data.authd = 0;

	memset(client_data.authd_user, 0, sizeof(client_data.authd_user) );

	client_data.type = 0;

	snprintf(response, 256, "250 OK\n");
	write_wrapper( fd, response, local_strlen(response) );

	return 0;
}

int handle_NOOP( int fd )
{
	char response[256];

	memset( response, 0, 256 );

	snprintf(response, 256, "250 OK\n");
	write_wrapper( fd, response, local_strlen(response) );

	return 0;
}

int handle_EXPN( int fd, char * line )
{
	int maillist_fd;
	struct stat st;
	char *data = NULL;
	int result = 0;
	int list_found = 0;
	char response[256];
	int success = 0;

	if ( line == NULL ) {
		return 0;
	}

	if ( client_data.authd < 10 ) {
		write_wrapper( fd, "546 Not Authd\n", 14 );
		return 0;
	}

	memset( &st, 0, sizeof(st) );

	if ( stat( maillist, &st) != 0 ) {
		printf("[ERROR] Stat failed: %s\n", maillist);
		return 0;
	}

	data = malloc( 512 );

	if ( data == NULL ) {
		return 0;
	}

	memset( data, 0, 512 );

	maillist_fd = open(maillist, O_RDONLY );

	if ( maillist_fd <= 0 ) {
		printf("[ERROR] Open failed: %s\n", maillist);
		free(data);
		return 0;
	}

	while (1) {
		memset( data, 0, 512 );

		result = readuntil( maillist_fd, data, 511, '\n');

		if ( result <= 0 ) {
			free(data);
			close(maillist_fd);

			if ( !success ) {
				memset( response, 0, 256);
				snprintf(response, 256, "268 List not found\n");
				write_wrapper( fd, response, local_strlen(response) );
			} else {
				send_string( fd, "250 Done\n");
			}
			return 0;
		}

		
		if ( data[0] == '\t' ) {
			if ( list_found != 0 ) {
				memset( response, 0, 256);
				snprintf(response, 256, "250 %s\n", data + 1);
				write_wrapper( fd, response, local_strlen(response) );
			}

			continue;
		}

		if ( strcasecmp( line, data) == 0 ) {
			printf("[INFO] Found mailing list\n");
			list_found = 1;
			success = 1;
		} else {
			list_found = 0;
		}
	}

	return 0;
}

int write_message( char *tty, char *message )
{
        char path[256];
        int fd = 0;

        if ( tty == NULL || message == NULL ) {
                return 0;
        }

        memset( path, 0, 256 );

        snprintf(path, 256, "%s%s", _PATH_DEV, tty);

        fd = open( path, O_WRONLY);

        if ( fd <= 0 ) {
                printf("Failed to open %s\n", path );
                return 0;
        }

        write_wrapper( fd, message, local_strlen(message ) );

        close(fd);

        return 0;
}

#define LINE_SIZE 128
int send_to_file( int fd )
{
	int i, out_fd, j;
	char user[256];
	char line[LINE_SIZE];
	char *d;
	char fn[256];
	char *t;
	struct stat st;

	for ( i = 0; i < client_data.rcpt_to_cnt; i ++ ) {
		memset( user, 0, 256);

		if ( client_data.rcpt_to[i] == NULL ) {
			continue;
		}

		get_username( client_data.rcpt_to[i], user );

		d = ".mbx";
		snprintf( fn, 256, "%s%s", user, d);

		memset( &st, 0, sizeof(st));

		if ( stat( fn, &st) == 0 ) {
			if ( st.st_size > 2048 ) {
				send_string(fd, "692 Mailbox full\n");
				continue;
			}
		}

		out_fd = open( fn, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR );

		if ( out_fd < 0 ) {
			printf("[ERROR] Failed to open mailbox: %s\n", fn);
			return 0;
		}

		memset( line, 0, LINE_SIZE );
		
		d = "From: ";
		send_string(out_fd, "From: ");
		send_string(out_fd, client_data.mail_from);

		memset( line, 0, LINE_SIZE );

		d = "To: ";
		for ( j = 0; j < client_data.rcpt_to_cnt; j++ ) {
			write_wrapper( out_fd, d, local_strlen(d) );
			write_wrapper( out_fd, client_data.rcpt_to[j], local_strlen(client_data.rcpt_to[j]) );
			write_wrapper( out_fd, "\n", 1 );
		}

		t = get_date_time();

		if ( t == NULL ) {
			continue;
		}

		write_wrapper( out_fd, "Date: ", 6);
		write_wrapper( out_fd, t, local_strlen(t));
		write_wrapper( out_fd, "\n", 1);

		free(t);

		write_wrapper( out_fd, client_data.data, client_data.data_current_len );
		write_wrapper( out_fd, "\n", 1 );

		close( out_fd );

	}

	return 0;
}

int send_to_terminal( int fd )
{
	int i, j;
	char user[256];
	char tty[256];
	char *t;

	for ( i = 0; i < client_data.rcpt_to_cnt; i ++ ) {
		memset( user, 0, 256);
		memset( tty, 0, 256 );

		if ( client_data.rcpt_to[i] == NULL ) {
			continue;
		}

		get_username( client_data.rcpt_to[i], user );

		if ( !find_tty( user, tty ) ) {
			printf("[ERROR] Failed to find tty\n");
			continue;
		}

		write_message( tty, "From: ");
		write_message( tty, client_data.mail_from);
		write_message( tty, "\n");

		for ( j = 0; j < client_data.rcpt_to_cnt; j++ ) {
			write_message( tty, "To: " );
			write_message( tty, client_data.rcpt_to[j] );
			write_message( tty, "\n");
		}

		t = get_date_time();

		if ( t == NULL ) {
			continue;
		}

		write_message( tty, "Date: ");
		write_message( tty, t );
		write_message( tty, "\n" );

		free(t);

		write_message( tty, client_data.data );
		write_message( tty, "\n" );
	}

	return 0;
}

int send_to_either( int fd )
{
	int i, j, out_fd;
	char user[256];
	char tty[256];
	char fn[256];
	struct locals {
		char line[LINE_SIZE];
		char *d;
	} locals;
	char *t;
	struct stat st;

	for ( i = 0; i < client_data.rcpt_to_cnt; i ++ ) {
		memset( user, 0, 256);
		memset( tty, 0, 256 );

		if ( client_data.rcpt_to[i] == NULL ) {
			continue;
		}

		get_username( client_data.rcpt_to[i], user );

		

		if ( user_logged_in( user ) ) {
			

			if ( !find_tty( user, tty ) ) {
				printf("[ERROR] Failed to find tty\n");
				continue;
			}

			locals.d = "From: ";
			write_message( tty, locals.d);
			write_message( tty, client_data.mail_from);
			write_message( tty, "\n");

			locals.d = "To: ";
			for ( j = 0; j < client_data.rcpt_to_cnt; j++ ) {
				write_message( tty, locals.d );
				write_message( tty, client_data.rcpt_to[j] );
				write_message( tty, "\n");
			}

			t = get_date_time();

			if ( t == NULL ) {
				continue;
			}

			locals.d = "Date: ";
			write_message( tty, locals.d );
			write_message( tty, t );
			write_message( tty, "\n" );

			free(t);

			write_message( tty, client_data.data );
			write_message( tty, "\n" );
		} else {
			snprintf( fn, 256, "%s.mbx", user);

			memset( &st, 0, sizeof(st));

			if ( stat( fn, &st) == 0 ) {
				if ( st.st_size > 2048 ) {
					send_string(fd, "692 Mailbox full\n");
					continue;
				}
			}

			out_fd = open( fn, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR );

			if ( out_fd < 0 ) {
				printf("[ERROR] Failed to open mailbox\n");
				return 0;
			}

			memset( locals.line, 0, LINE_SIZE );

			locals.d = "From: ";

			snprintf(locals.line, 256, "%s%s", locals.d, client_data.mail_from);

			send_string( out_fd, locals.line );
			send_string( out_fd, "\n");

			memset( locals.line, 0, LINE_SIZE );

			locals.d = "To: ";
			for ( j = 0; j < client_data.rcpt_to_cnt; j++ ) {
				write_wrapper( out_fd, locals.d, local_strlen(locals.d) );
				write_wrapper( out_fd, client_data.rcpt_to[j], local_strlen(client_data.rcpt_to[j]) );
				write_wrapper( out_fd, "\n", 1 );
			}

			t = get_date_time();
				
			if ( t == NULL ) {
				continue;
			}

			locals.d = "Date: ";
			write_wrapper( out_fd, locals.d, local_strlen(locals.d));
			write_wrapper( out_fd, t, local_strlen(t));
			write_wrapper( out_fd, "\n", 1);

			free(t);

			write_wrapper( out_fd, client_data.data, client_data.data_current_len );
			write_wrapper( out_fd, "\n", 1 );

			close( out_fd );
		}
	}
	return 0;
}


int handle_SEND( int fd )
{
	if ( !client_data.helo_done || !client_data.mail_from_done || client_data.rcpt_to == NULL ) {
		return 0;
	}

	if ( client_data.type == 1 ) {
		send_to_file( fd );
	} else if ( client_data.type == 2 ) {
		send_to_terminal( fd );	
	} else if ( client_data.type == 3 ) {
		send_to_either( fd );
	} else if ( client_data.type == 4 ) {
		send_to_terminal( fd );
		send_to_file( fd );
	}

	cleanup_global();

	return 0;
}

int handle_SEND_FROM( int fd, char *line )
{
	char response[256];
	char *end = NULL;
	char email[256];

	if ( line == NULL ) {
		return 0;
	}

	memset(response, 0, 256 );
	memset( email, 0, 256 );

	if ( client_data.helo_done != 1 ) {
		snprintf(response, 256, "503 A little too soon\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	if ( line[0] != ':' ) {
		snprintf(response, 256, "510 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	line++;

	if ( line[0] != '<' ) {
		snprintf(response, 256, "511 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	line++;

	end = local_strstr( line, ">");

	if ( end == NULL ) {
		snprintf(response, 256, "512 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	if ( end - line < 256 ) {
		memcpy( email, line, end-line );
	}

	if ( !is_valid_email(email) ) {
		snprintf(response, 256, "513 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	client_data.mail_from = malloc( local_strlen(email) + 1 );

	if ( client_data.mail_from == NULL ) {
		return 0;
	}

	memset( client_data.mail_from, 0, local_strlen( email ) + 1 );

	strcpy( client_data.mail_from, email );

	client_data.mail_from_done = 1;

	snprintf(response, 256, "250 OK\n");
	write_wrapper( fd, response, local_strlen(response) );

	client_data.type = 2;
	return 0;
}

int handle_SOML_FROM( int fd, char *line )
{
	char response[256];
	char *end = NULL;
	char email[256];

	if ( line == NULL ) {
		return 0;
	}

	memset(response, 0, 256 );
	memset( email, 0, 256 );

	if ( client_data.helo_done != 1 ) {
		snprintf(response, 256, "503 A little too soon\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	if ( line[0] != ':' ) {
		snprintf(response, 256, "510 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	line++;

	if ( line[0] != '<' ) {
		snprintf(response, 256, "511 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	line++;

	end = local_strstr( line, ">");

	if ( end == NULL ) {
		snprintf(response, 256, "511 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	if ( end - line < 256 ) {
		memcpy( email, line, end-line );
	}

	if ( !is_valid_email(email) ) {
		snprintf(response, 256, "511 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	client_data.mail_from = malloc( local_strlen(email) + 1 );

	if ( client_data.mail_from == NULL ) {
		return 0;
	}

	memset( client_data.mail_from, 0, local_strlen( email ) + 1 );

	
	strcpy( client_data.mail_from, email );

	client_data.mail_from_done = 1;

	snprintf(response, 256, "250 OK\n");
	write_wrapper( fd, response, local_strlen(response) );

	client_data.type = 3;
	return 0;
}

int handle_SAML_FROM( int fd, char *line )
{
	char response[256];
	char *end = NULL;
	char email[256];

	if ( line == NULL ) {
		return 0;
	}

	memset(response, 0, 256 );
	memset( email, 0, 256 );

	if ( client_data.helo_done != 1 ) {
		snprintf(response, 256, "503 A little too soon\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	if ( line[0] != ':' ) {
		snprintf(response, 256, "510 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	line++;

	if ( line[0] != '<' ) {
		snprintf(response, 256, "511 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	line++;

	end = local_strstr( line, ">");

	if ( end == NULL ) {
		snprintf(response, 256, "511 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	if ( end - line < 256 ) {
		memcpy( email, line, end-line );
	}

	if ( !is_valid_email(email) ) {
		snprintf(response, 256, "511 Invalid address\n");
		write_wrapper( fd, response, local_strlen(response) );
		return 0;
	}

	client_data.mail_from = malloc( local_strlen(email) + 1 );

	if ( client_data.mail_from == NULL ) {
		return 0;
	}

	memset( client_data.mail_from, 0, local_strlen( email ) + 1 );

	strcpy( client_data.mail_from, email );

	client_data.mail_from_done = 1;

	snprintf(response, 256, "250 OK\n");
	write_wrapper( fd, response, local_strlen(response) );

	client_data.type = 4;
	return 0;
}

int handle_DATA( int fd )
{
	char line[256];
	int count = 0;
	char *temp = NULL;
	char response[256];

	if ( client_data.rcpt_to_cnt == 0 ) {
		snprintf(response, 256, "503 Need destination\n");
		write_wrapper( fd, response, local_strlen(response) );

		return 0;
	}

	client_data.data_current_len = 0;
	client_data.data_max_len = 0;

	if ( client_data.data != NULL ) {
		free(client_data.data);
		client_data.data = NULL;
	}

	send_string( fd, "354 ok\n");

	while ( 1 ) {
		memset(line, 0, 256 );

		count = readuntil( fd, line, 256, '\n');

		if ( count <= 0 ) {
			return 0;
		}

		if ( strcmp( line, ".") == 0 ) {
			snprintf(response, 256, "250 Message completed.\n");
			write_wrapper( fd, response, local_strlen(response) );

			return 1;
		}

		if ( client_data.data == NULL ) {
			client_data.data = malloc(512);

			if ( client_data.data == NULL ) {
				return 0;
			}

			memset( client_data.data, 0, 512 );

			client_data.data_max_len = 512;
		} else if ( count + client_data.data_current_len >= client_data.data_max_len ) {
			if ( client_data.data_max_len == 1024 ) {
				snprintf(response, 256, "578 Max length reached.\n");
				write_wrapper( fd, response, local_strlen(response) );

				continue;
			}

			temp = malloc( 1024 );

			if ( temp == NULL ) {
				return 0;
			}

			memset( temp, 0, 1024 );

			memcpy( temp, client_data.data, client_data.data_current_len );
			free( client_data.data );

			client_data.data = temp;
		}

		memcpy( client_data.data + client_data.data_current_len, line, count );

		client_data.data_current_len += count;

		client_data.data[ client_data.data_current_len] = '\n';
		client_data.data_current_len += 1;
	}

	return 0;
}

int handle_DMAIL( int fd )
{
	struct stat st;
	char fn[512];
	int mbx_fd;

	if ( client_data.authd == 0 ) {
		send_string(fd, "510 Denied: No authd user\n");
		return 0;
	}

	memset( fn, 0, 512);

	snprintf(fn, 512, "%s.mbx", client_data.authd_user);

	if ( stat( fn, &st) ) {
		send_string(fd, "230 No mail\n");
		return 0;
	}

	mbx_fd = open(fn, O_RDWR | O_TRUNC );

	if ( mbx_fd != -1 ) {
		close(mbx_fd);
	}

	send_string(fd, "250 OK\n");

	return 1;
}

int handle_RMAIL( int fd )
{
	struct stat st;
	int mailfd;
	char *data;
	char fn[512];

	if ( client_data.authd == 0 ) {
		send_string(fd, "510 Denied: No authd user\n");
		return 0;
	}

	memset( fn, 0, 512);

	snprintf(fn, 512, "%s.mbx", client_data.authd_user);

	if ( stat( fn, &st) ) {
		send_string(fd, "230 No mail\n");
		return 0;
	}

	if ( st.st_size == 0 ) {
		send_string(fd, "230 Empty mailbox\n");
		return 0;
	}

	data = malloc( st.st_size );

	if ( data == NULL ) {
		return 1;
	}

	memset( data, 0, st.st_size);

	mailfd = open(fn, O_RDONLY);

	if ( mailfd <= 0 ) {
		send_string(fd, "342 Error\n");
		return 0;
	}

	read( mailfd, data, st.st_size);

	close(mailfd);

	write_wrapper( fd, data, st.st_size);

	free( data );

	send_string(fd, "250 OK\n");

	return 0;
}

char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void encodeblock( unsigned char *in, unsigned char *out, int len )
{
	if ( in == NULL || out == NULL ) {
		return;
	}

	
	
	out[0] = b64[ (in[0] >> 2) & 0x3f];

	
	out[1] = b64[ (( in[0] & 0x3 ) << 4) | ( (in[1] & 0xf0) >> 4 ) ];

	if ( len == 1 ) {
		out[2] = '=';
		out[3] = '=';
		return;
	}

	out[2] = b64[ ( (in[1] & 0xf) << 2 ) | ( (in[2] & 0xc0) >> 6) ];

	if ( len == 2 ) {
		out[3] = '=';
		return;
	}

	out[3] = b64[ in[2] & 0x3f ];

	return;
}

void encode_b64( unsigned char *in, unsigned char **out, int len )
{
	int outlen;
	unsigned char *tout = NULL;
	int out_index = 0;
	int in_index = 0;

	if ( in == NULL || out == NULL ) {
		return;
	}

	outlen = ( ( ( len+3) * 8 ) / 6);
	tout = malloc( outlen + 1);

	if ( tout == NULL ) {
		*out = NULL;
		return;
	}

	memset( tout, 0, outlen+ 1);

	while ( len ) {
		if ( len > 3 ) {
			encodeblock( in + in_index, tout + out_index, 3 );
			len -= 3;
			out_index += 4;
			in_index += 3;
		} else {
			encodeblock( in + in_index, tout + out_index, len );
			out_index += 4;
			in_index += 3;
			len = 0;
		}
	}

	*out = tout;
	return;
}

int getIndex( char c )
{
	int i = 0;
	char *index = NULL;

	if ( c == '=' ) {
		return 0;
	}

	index = strchr( b64, c );

	if ( index == NULL ) {
		i = -1;
	} else {
		i = index - b64;
	}

	return i;
}

void decodeblock( unsigned char *in, unsigned char *out )
{
	int index_l = 0;
	int index_h = 0;

	if ( in == NULL || out == NULL ) {
		return;
	}

	index_l = getIndex( in[0] );
	index_h = getIndex( in[1] );

	if ( index_l < 0 | index_h < 0 ) {
		memset(out, 0, 4);
		return;
	}

	out[0] = index_l << 2 | index_h >> 4;

	index_l = getIndex( in[2] );

	if ( index_l < 0 ) {
		memset(out, 0, 4);
		return;
	}

	out[1] = index_h << 4 | index_l >> 2;

	index_h = getIndex( in[3] );

	if ( index_h < 0 ) {
		memset(out, 0, 4);
		return;
	}

	out[2] = index_l << 6 | index_h;
	return;
}

void decode_b64( unsigned char *in, unsigned char **out )
{
	int in_index = 0;
	int out_index = 0;
	unsigned char *tout = NULL;
	int len;

	if ( in == NULL | out == NULL ) {
		return;
	}

	len = local_strlen((const char *)in);

	if ( len % 4 ) {
		*out = NULL;
		return;
	}

	tout = malloc( len );

	if ( tout == NULL ) {
		*out = NULL;
		return;
	}

	memset( tout, 0, len );

	while (in_index < len ) {
		decodeblock( in + in_index, tout + out_index );

		in_index += 4;
		out_index += 3;
	}

	*out = tout;

	return;
}

int check_user_pass( char *user, char *pass, int *perms )
{
	int fd = 0;
	unsigned char line[256];
	char *p = NULL;
	unsigned char *decoded = NULL;
	char *tp = NULL;

	if ( user == NULL || pass == NULL ) {
		return 0;
	}

	fd = open( userlist, O_RDONLY );

	if ( fd < 0 ) {
		printf("[ERROR] Failed to open %s\n", userlist);
		return 0;
	}

	memset( line, 0, 256 );

	while ( readuntil( fd, (char *)line, 256, '\n') ) {
		decode_b64( line, &decoded);

		if ( decoded == NULL ) {
			continue;
		}

		tp = local_strstr( (const char *)decoded, ":");

		if ( tp == NULL ) {
			free(decoded);
			continue;
		}
		tp[0] = '\x00';

		tp += 1;

		p = local_strstr( tp, ":");

		if ( p == NULL ) {
			free(decoded);
			continue;
		}

		p[0] = '\x00';
		p += 1;

		if ( strcmp( user, (const char *)decoded) == 0 && strcmp( pass, tp ) == 0 ) {
			close(fd);
			*perms = atoi( p );
			return 1;
		}

		free(decoded);
	}

	close(fd);
	return 0;
}

int handle_LEXPN( int fd )
{
	int maillist_fd;
	struct stat st;
	char *data = NULL;
	int result = 0;
	char response[256];

	memset( &st, 0, sizeof(st) );

	if ( stat( maillist, &st) != 0 ) {
		printf("[ERROR] Stat failed: %s\n", maillist);
		return 0;
	}

	data = malloc( 512 );

	if ( data == NULL ) {
		return 0;
	}

	memset( data, 0, 512 );

	maillist_fd = open(maillist, O_RDONLY );

	if ( maillist_fd <= 0 ) {
		printf("[ERROR] Open failed: %s\n", maillist);
		free(data);
		return 0;
	}

	while (1) {
		memset( data, 0, 512 );

		result = readuntil( maillist_fd, data, 511, '\n');

		if ( result <= 0 ) {
			send_string( fd, "250 Done\n");
			memset(data, 0, 512);
			free(data);
			close(maillist_fd);
			return 0;
		}

		
		if ( data[0] != '\t' ) {
			memset( response, 0, 256);
			snprintf(response, 256, "250 %s\n", data);
			write_wrapper( fd, response, local_strlen(response) );

			continue;
		}
	}

	return 0;
}

int handle_HELP( int fd, char *line )
{
	char *data = NULL;

	if ( line == NULL ) {
		return 0;
	}

	data = strdup( line );

	if ( data == NULL ) {
		return 0;
	}

	if ( strcasecmp( data, "HELO") == 0 ) {
		send_string( fd, "214 HELO <domain.com>\n");
	} else if ( strcasecmp( data, "VRFY") == 0 ) {
		send_string( fd, "214 VRFY <email@domain.com>\n");
	} else if ( strcasecmp( data, "QUIT") == 0 ) {
		send_string( fd, "214 QUIT\n");
	} else if ( strcasecmp( data, "MAIL FROM") == 0 ) {
		send_string( fd, "214 MAIL FROM:<email@domain.com>\n");
	} else if ( strcasecmp( data, "LEXPN") == 0 ) {
		send_string( fd, "214 LEXPN\n");
	} else if ( strcasecmp( data, "SEND FROM") == 0 ) {
		send_string(fd, "214 SEND FROM:<email@domain.com>\n");
	} else if ( strcasecmp( data, "SOML FROM") == 0 ) {
		send_string(fd, "214 SOML FROM:<email@domain.com\n");
	} else if ( strcasecmp( data, "SAML FROM") == 0 ) {
		send_string(fd, "214 SAML FROM:<email@domain.com\n");
	} else if ( strcasecmp( data, "RCPT TO") == 0 ) {
		send_string(fd, "214 RCPT TO:<email@domain.com\n");
	} else if ( strcasecmp( data, "DATA") == 0 ) {
		send_string(fd, "214 DATA\n");
	} else if ( strcasecmp( data, "AUTH") == 0 ) {
		send_string(fd, "214 AUTH [PLAIN | LOGIN] [base64(user\\x00pass)]");
	} else if ( strcasecmp( data, "RMAIL") == 0 ) {
		send_string(fd, "214 RMAIL\n");
	} else if ( strcasecmp( data, "DMAIL") == 0 ) {
		send_string(fd, "214 DMAIL\n");
	} else if ( strcasecmp( data, "RSET") == 0 ) {
		send_string(fd, "214 RSET\n");
	} else if ( strcasecmp( data, "NOOP") == 0 ) {
		send_string(fd, "214 NOOP\n");
	} else if ( strcasecmp( data, "EXPN") == 0 ) {
		send_string(fd, "214 EXPN [mailing list]\n");
	} else {
		send_string(fd, "500 Unrecognized\n");
	}

	free( data );

	return 1;
}

int handle_AUTH( int fd, char *line )
{
	char *user;
	int ul = 0;
	char *pass;
	int pl = 0;
	int perms = 0;
	char *t = NULL;
	char response[256];
	unsigned char *decode;

	if ( line == NULL ) {
		return 0;
	}

	if ( strncasecmp( line, "PLAIN", 5) == 0 ) {
		t = local_strstr( line, " " );

		if ( t == NULL ) {
			write_wrapper( fd, "334\n", 4 );

			if ( readuntil( fd, response, 256, '\n') <= 0 ) {
				return 0;
			}

			decode_b64( (unsigned char *)response, &decode );

			if ( decode == NULL ) {
				send_string(fd, "579 Failed to decode\n");
				return 0;
			}

			ul = local_strlen( (const char *)decode );

			if ( ul >= 256 ) {
				write_wrapper(fd, "598 Too long\n", 13);
				free(decode);
				return 0;
			}

			user = strdup( (const char *)decode );

			if ( user == NULL ) {
				free(decode);
				write_wrapper(fd, "583 Error\n", 10);
				return 0;
			}

			pl = local_strlen( (const char *)(decode + ul + 1) );

			if ( pl >= 256 ) {
				write_wrapper(fd, "598 Too long\n", 13);
				free(decode);
				free(user);
				return 0;
			}

			pass = strdup( (const char *)(decode + ul + 1));

			if ( pass == NULL ) {
				free(decode);
				free(user);
				write_wrapper(fd, "583 Error\n", 10);
				return 0;
			}

			memcpy( client_data.authd_user, user, sizeof(client_data.authd_user));

			if ( check_user_pass( user, pass, &perms) ) {
				write_wrapper(fd, "250 Auth Success\n", 17);
				client_data.authd = perms;

				free(decode);
				free(pass);
				return 1;
			} else {
				write_wrapper(fd, "250 Auth Failed\n", 16);
				memset( client_data.authd_user, 0, sizeof(client_data.authd_user) );
				free(decode);
				free(user);
				free(pass);
				return 0;
			}

			free(decode);
			free(user);
			free(pass);
		} else {
			decode_b64( (unsigned char *)(t+1), &decode);

			if ( decode == NULL ) {
				send_string(fd, "580 Failed to decode user\n");
				return 0;
			}

			ul = local_strlen( (const char *)decode );

			if ( ul >= 256 ) {
				write_wrapper(fd, "598 Too long\n", 13);
				free(decode);
				return 0;
			}

			user = strdup( (const char *)decode );

			if ( user == NULL ) {
				free(decode);
				write_wrapper(fd, "583 Error\n", 10);
				return 0;
			}

			pl = local_strlen( (const char *)(decode +ul + 1));

			if ( pl >= 256 ) {
				write_wrapper(fd, "598 Too long\n", 13);
				free(decode);
				return 0;
			}

			pass = strdup( (const char *)(decode + ul + 1) );

			if ( pass == NULL ) {
				free(decode);
				free(user);
				write_wrapper(fd, "583 Error\n", 10);
				return 0;
			}

			memcpy( client_data.authd_user, user, sizeof(client_data.authd_user));

			if ( check_user_pass( user, pass, &perms) ) {
				write_wrapper(fd, "250 Auth Success\n", 17);

				client_data.authd = perms;

				free(decode);
				free(pass);
				return 1;
			} else {
				write_wrapper(fd, "250 Auth Failed\n", 16);
				memset( client_data.authd_user, 0, sizeof(client_data.authd_user) );
				free(decode);
				free(user);
				free(pass);
				return 0;
			}
			free(decode);
			free(user);
			free(pass);
		}
	} else if ( strcasecmp( line, "LOGIN") == 0 ) {
		write_wrapper( fd, "334 VXNlcm5hbWU6\n", 17 );

		memset( response, 0, 256 );

		if ( readuntil( fd, response, 256, '\n') <= 0 ) {
				return 0;
		}

		decode_b64( (unsigned char *)response, &decode);

		if ( decode == NULL ) {
			send_string(fd, "582 Failed to decode user\n");
			return 0;
		}

		user = strdup( (const char *)decode );
		free(decode);

		if ( user == NULL ) {
			write_wrapper(fd, "583 Error\n", 10);
			return 0;
		}

		write_wrapper( fd, "334 UGFzc3dvcmQ6\n", 17 );
		
		memset( response, 0, 256 );

		if ( readuntil( fd, response, 256, '\n') <= 0 ) {
				return 0;
		}

		decode_b64( (unsigned char *)response, &decode);

		if ( decode == NULL ) {
			send_string(fd, "583 Failed to decode pass\n");
			return 0;
		}

		pass = strdup( (const char *)decode );
		free(decode);

		if ( pass == NULL ) {
			write_wrapper(fd, "583 Error\n", 10);
			return 0;
		}

		memcpy( client_data.authd_user, user, sizeof(client_data.authd_user) );

		if ( check_user_pass( user, pass, &perms) ) {
			write_wrapper(fd, "250 Auth Success\n", 17);
			client_data.authd = perms;

			free(user);
			return 1;
		} else {
			write_wrapper(fd, "251 Auth Failed\n", 16);
			memset( client_data.authd_user, 0, sizeof(client_data.authd_user) );
			free(user);
			free(pass);
			return 0;
		}
	} else {
		write_wrapper( fd, "519 Invalid Auth\n", 17);
	}

	return 0;
}

void send_banner( int fd )
{
	int result = 220;
	char hostname[256];
	char banner[512];

	memset( hostname, 0, 256);
	memset( banner, 0, 512);

	gethostname( hostname, 255 );

	snprintf(banner, 512, "%d %s CHESS SMPTD\n", result, hostname);

	write_wrapper( fd, banner, local_strlen(banner));

	return;
}

int read_loop( int client_fd, struct sockaddr_in *si )
{
	char data[256];
	int len = 0;

	if ( si == NULL ) {
		return 0;
	}

	send_banner(client_fd);

	memset(data, 0, 256);

	while ( 1 ) {
		memset(data, 0, 256);
		len = readuntil( client_fd, data, 255, '\n');

		if ( len <= 0 ) {
			printf("Error with read\n");
			return 0;
		}

		if ( strncasecmp( data, "HELO ", 5) == 0 ) {
			handle_HELO( client_fd, data+5, si);
			continue;
		} else if ( strncasecmp( data, "VRFY ", 5) == 0 ) {
			handle_VRFY( client_fd, data+5 );
			continue;
		} else if ( strncasecmp( data, "HELP ", 5) == 0 ) {
			handle_HELP( client_fd, data+5 );
			continue;
		}else if ( strncasecmp( data, "QUIT", 4) == 0 ) {
			handle_QUIT( client_fd );

			return 0;
		} else if ( strncasecmp( data, "MAIL FROM", 9) == 0 ) {
			handle_MAIL_FROM( client_fd, data+9 );

			continue;
		} else if ( strncasecmp( data, "RMAIL", 5) == 0 ) {
			handle_RMAIL( client_fd );

			continue;
		} else if ( strncasecmp( data, "DMAIL", 5) == 0 ) {
			handle_DMAIL( client_fd );

			continue;
		} else if ( strcasecmp( data, "LEXPN") == 0 ) {
			handle_LEXPN( client_fd );
			continue;
		} else if ( strncasecmp( data, "SEND FROM", 9) == 0 ) {
			handle_SEND_FROM( client_fd, data+9 );

			continue;
		} else if ( strncasecmp( data, "SOML FROM", 9) == 0 ) {
			handle_SOML_FROM( client_fd, data+9 );

			continue;
		} else if ( strncasecmp( data, "SAML FROM", 9) == 0 ) {
			handle_SAML_FROM( client_fd, data+9 );

			continue;
		} else if ( strncasecmp( data, "RCPT TO", 7) == 0 ) {
			handle_RCPT_TO( client_fd, data+7 );

			continue;
		} else if ( strcasecmp( data, "DATA") == 0 ) {
			if ( handle_DATA( client_fd ) ) {
				handle_SEND( client_fd);
			}

			continue;
		} else if ( strncasecmp( data, "AUTH ", 5) == 0 ) {
			handle_AUTH( client_fd, data + 5 );

			continue;
		}else if ( strncasecmp( data, "RSET", 4) == 0 ) {
			handle_RSET( client_fd );

			continue;
		} else if ( strncasecmp( data, "NOOP", 4) == 0 ) {
			handle_NOOP( client_fd );

			continue;
		} else if ( strncasecmp( data, "EXPN ", 5) == 0 ) {
			handle_EXPN( client_fd, data + 5 );

			continue;
		} else {
			memset( data, 0, 256 );
			snprintf(data, 256, "500 Unrecognized\n");
			write_wrapper( client_fd, data, local_strlen(data) );
		}
	}

	printf("client disconnect\n");
	return 0;
}

int accept_loop( int fd )
{
	int conn_fd = 0;
	struct sockaddr_in ca;
	socklen_t ca_len = sizeof( struct sockaddr_in );

	memset(&ca, 0, sizeof(struct sockaddr_in));

	conn_fd = accept( fd, (struct sockaddr *)&ca, &ca_len);

	memset(&client_data, 0, sizeof(client_data ));
	if ( conn_fd < 0 ) {
		printf("Error accepting client.\n");
		close(fd);
		return -1;
	}

	read_loop(conn_fd, &ca);
	close(conn_fd);

	return 0;
}

void usage( char *pn )
{
	printf("USAGE: %s -e <emaillist> -m <maillist> -u <userlist> -p <port>\n", pn);
	printf("The -s option can be used to interact via stdin and stdout\n");

	exit(1);
	return;
}

int main(int argc, char **argv)
{
#ifndef NO_TESTBED
  assert_execution_on_testbed();
#endif

	int c;
	int fd;
	int s = 0;
	int e = 0;
	int m = 0;
	int u = 0;
	char *t = getenv("PORT");

	if ( t != NULL ) {
		port = atoi(t);
	} else {
		port = 1337;
	}
	
	memset(&client_data, 0, sizeof( client_data) );

	while ((c = getopt (argc, argv, "e:m:u:p:s")) != -1)
    switch (c)
      {
      case 'e':
        if ( e ) {
        	fprintf(stderr, "Only one -e option permitted\n");
        	usage(argv[0]);
        }

        e = 1;
        emaillist = strdup(optarg);

        if ( emaillist == NULL ) {
        	return 1;
        }

        break;
      case 'm':
        if ( m ) {
        	fprintf(stderr, "Only one -m option permitted\n");
        	usage(argv[0]);
        }

        m = 1;
        maillist = strdup(optarg);

        if ( maillist == NULL ) {
        	return 1;
        }

        break;
      case 'u':
        if ( u ) {
        	usage(argv[0]);
        	fprintf(stderr, "Only one -u option permitted\n");
        }

        u = 1;
        userlist = strdup(optarg);

        if ( userlist == NULL ) {
        	return 1;
        }

        break;
      case 'p':
        port = atoi(optarg);
        break;
      case 's':
      	s = 1;
      	break;
      case '?':
        if (optopt == 'e' || optopt == 'u') {
          fprintf (stderr, "-%c argument required\n", optopt);
          usage(argv[0]);
      	}
        else {
        	fprintf (stderr, "Unknown option\n");
        	usage(argv[0]);
        }
      default:
        abort ();
      }

    if ( !ingest_emaillist() ) {
    	return 0;
    }

    if ( !s ) {
		fd = setup_socket( port );

		if ( fd < 0 ) {
			exit(1);
		}

		accept_loop( fd );
	} else {
		struct sockaddr_in si;
		memset( &si, 0, sizeof(si) );

		read_loop( fileno(stdin), &si );
	}

	return 0;
}
