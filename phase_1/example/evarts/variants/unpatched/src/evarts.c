
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>

#include "testbed.h"

#define MAX_CACHE (256)
#define MAX_INT_DATA (10)
#define MAX_LLINT_DATA (19)
#define MAX_DATA (16)
#define MAX_NAME (64)
#define MAX_TABLES (1)
#define MAX_COLUMNS (2)
#define MAX_ROWS (30)
#define MAX_INGREDIENTS (200)
#define SQL_VAR_TYPE(a) (((sqlvar*)NULL)->a)
#define SQL_DB_TYPE(a) (((sqldb*)NULL)->a)
#define SQL_TABLE_TYPE(a) (((sqltable*)NULL)->a)
#define SQL_NUM_ELEMENTS(a) (sizeof(a) / sizeof(a[0]))
#define SQL_CACHE_HIT_JIT 5

typedef struct sqlexec sqlexec;
typedef struct sqldb sqldb;
typedef struct sqlvarlist sqlvarlist;
typedef struct sqlstmt sqlstmt;
typedef struct sqlvarstmt sqlvarstmt;
typedef struct sqlerrorstmt sqlerrorstmt;
typedef struct sqlexp sqlexp;
typedef struct sqlnode sqlnode;
typedef struct sqltree sqltree;

#define SQL_NULL    (0)
#define SQL_INT     (1)
#define SQL_FLOAT   (2)
#define SQL_REAL    (3)
#define SQL_VARCHAR (4)
#define SQL_BIGINT  (5)

typedef uint32_t sqltype;
typedef uint32_t sqlsize;

typedef enum sqlboolean {
	SQL_BOOLEAN_NONE,
	SQL_BOOLEAN_OR,
	SQL_BOOLEAN_AND,
} sqlboolean;

typedef enum sqllogic {
	SQL_CONDITION_EQ,
	SQL_CONDITION_NE,
	SQL_CONDITION_LT,
	SQL_CONDITION_LTE,
	SQL_CONDITION_GT,
	SQL_CONDITION_GTE
} sqllogic;

typedef enum sqlarithmetic {
	SQL_ARITHMETIC_ADD,
	SQL_ARITHMETIC_SUB,
	SQL_ARITHMETIC_MUL,
	SQL_ARITHMETIC_DIV,
	SQL_ARITHMETIC_NONE
} sqlarithmetic;

typedef sqlvarlist *(*sqlaction)(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *vars);
typedef const char *(*sqlparser)(sqltype *t, const char *s);
typedef void (*menu_action)(void);

static sqlexp *parser_wspace_ignore = NULL;
static sqlexp *parser_table_literal = NULL;
static sqlexp *parser_var_literal = NULL;
static sqlexp *parser_var_float = NULL;
static sqlexp *parser_var_integer = NULL;
static sqlexp *parser_var_arg = NULL;
static sqlexp *parser_list_operator = NULL;
static sqlexp *parser_assignment_operator = NULL;
static sqlexp *parser_equal_operator = NULL;
static sqlexp *parser_notequal_operator = NULL;
static sqlexp *parser_notequal2_operator = NULL;
static sqlexp *parser_less_operator = NULL;
static sqlexp *parser_greater_operator = NULL;
static sqlexp *parser_lessequal_operator = NULL;
static sqlexp *parser_greaterequal_operator = NULL;
static sqlexp *parser_leftparenthesis_operator = NULL;
static sqlexp *parser_rightparenthesis_operator = NULL;
static sqlexp *parser_and_logical = NULL;
static sqlexp *parser_or_logical = NULL;
static sqlexp *parser_not_logical = NULL;
static sqlexp *parser_at_operator = NULL;
static sqlexp *parser_wildcard_operator = NULL;
static sqlexp *parser_assignmentlist_operator = NULL;
static sqlexp *parser_add_arithmetic = NULL;
static sqlexp *parser_sub_arithmetic = NULL;

typedef struct sqltree {
	sqlnode *root;
} sqltree;

typedef struct sqlexp {
	sqlparser parser;
	sqlaction action;
	const char *match;
} sqlexp;

typedef struct sqlnode {
	sqlexp  *exp;
	sqlnode *args;
	sqlnode *next;
	size_t visit_ref;
	size_t commit_ref;
} sqlnode;

typedef struct sqlvar {
	union {
		uint8_t data[MAX_DATA];
		int idata;
		long long lldata;
		double fdata;
		float rdata;
		char sdata[MAX_DATA];
	};
	sqltype type;
	sqlsize size;
} sqlvar;

typedef struct sqllist {
	char id[MAX_NAME];
	size_t nvars;
	size_t next_list;
} sqllist;

typedef void(*sqllist_free)(sqllist *vlist);

typedef struct sqlvarlist {
	sqllist list;
	sqlvar *vars;
} sqlvarlist;

typedef struct sqlcolumnlist {
	sqllist list;
	sqltype datatype;
	sqlvar rows[MAX_ROWS];
} sqlcolumnlist;

typedef struct sqlptrlist {
	sqllist list;
	sqllist *vlist;
	sqlvar *vars;
	sqllist_free free_vlist;
} sqlptrlist;

typedef struct sqltable {
	bool used;
	char name[MAX_NAME];
	size_t ncols;
	size_t nrows;
	sqlcolumnlist columns[MAX_COLUMNS];
} sqltable;

typedef struct sqlcache {
	sqlstmt *stmt;
	size_t hits;
	bool jitted;
} sqlcache;

typedef struct sqldb {
	uint8_t stmt_hash;
	sqlcache stmt_cache[UCHAR_MAX];
	char name[MAX_NAME];
	size_t num_tables;
	sqltable tables[MAX_TABLES];
} sqldb;

typedef struct sqlconditional {
	sqlvar left;
	sqlvar right;
	sqllogic logic_operation;
	sqlboolean bool_operation;
	size_t next_condition;
} sqlconditional;

typedef struct sqlformula {
	sqlarithmetic arithmetic_operation;
	sqlvar left;
	struct sqlformula *right;
} sqlformula;

typedef struct sqlassignment {
	sqlvar left;
	sqlformula *right;
	size_t next_assignment;
} sqlassignment;

typedef struct sqlexec {
	sqldb *db;
	sqlvarlist *args;
	sqltable   *table;
	sqlptrlist *cols;
	sqlvarlist *values;
	sqlassignment* assignments;
	sqlformula* formulas;
	sqlconditional* conditions;
	sqlconditional* last_condition;
} sqlexec;

typedef struct sqlstmt {
	sqlaction action;
	sqlstmt *q;
} sqlstmt;

typedef struct sqlerrorstmt {
	sqlstmt base;
	const char *location;
	const char *error;
} sqlerrorstmt;

typedef struct sqlvarstmt {
	sqlstmt base;
	sqlvar var;
} sqlvarstmt;

typedef struct sqlentity {
	int server_fd;
	int client_fd;
	sqldb db;
	menu_action admin;
	menu_action add_ingredient;
	menu_action make_burger;
	menu_action show_ingredients;
} sqlentity;

static sqlparser sql_null_parser = (sqlparser)NULL;
static sqlaction sql_null_action = (sqlaction)NULL;
static sqltree sql_parse_tree;


void appmain_setup(void);
bool appmain_wait_for_client(void);
void appmain_reset_client(int errcode);
void appmain_send_to_client(char *format, ...);
void appmain_cleanup(void);
bool appmain_insert_ingredient(const char *name, size_t name_max, long long price);
bool appmain_update_ingredient(const char *name, size_t name_max, long long price);
bool appmain_add_to_price_ingredient(const char *name, size_t name_max, long long price);
bool appmain_sub_from_price_ingredient(const char *name, size_t name_max, long long price);
bool appmain_is_ingredient(const char *name, size_t name_max);

void show_menu(void);
void do_show_ingredients(void);
void do_add_ingredient(void);
void do_make_burger(void);
void do_admin(void);
int get_choice(void);
size_t get_line(char *str, size_t len);


void sql_setup(void);
void sql_db_create(sqldb *db, const char *name);
void sql_db_destroy(sqldb *db);
bool sql_table_create(sqldb *db, const char *name);
bool sql_table_column_create(sqldb *db, const char *table, const char *name, sqltype t);
void sql_destroy_table(sqldb *db, const char *name);

sqlvarlist *sql_query(sqldb *db, const char *query, sqlvarlist *args);
sqlvarlist *sql_query_nocache(sqldb *db, const char *query, sqlvarlist *args);

sqlvarlist *sql_list_new_varlist(const char *id);
void        sql_varlist_free(sqlvarlist *l);
sqlvarlist *sql_varlist_add_null(sqlvarlist *l);
sqlvarlist *sql_varlist_add_varchar(sqlvarlist *l, const void *data, sqlsize s);
sqlvarlist *sql_varlist_add_float(sqlvarlist *l, double data);
sqlvarlist *sql_varlist_add_real(sqlvarlist *l, float data);
sqlvarlist *sql_varlist_add_int(sqlvarlist *l, int data);
sqlvarlist *sql_varlist_add_bigint(sqlvarlist *l, long long data);
sqlvarlist *sql_varlist_add_varstruct(sqlvarlist *l, const sqlvar *v);
sqlvarlist *sql_varlist_append_varlist(sqlvarlist **l, const char *id);
void        appmain_print_ingredients(const sqlvarlist *l);

sqllist    *sql_list_get_next_list(const sqllist *l);
sqllist    *sql_list_get_list_id(sqllist *l, const char *id);

sqlsize     sql_strlen(const char *name, size_t size);


long long sql_var_get_bigint(const sqlvar *v);
int       sql_var_get_int(const sqlvar *v);
void      sql_var_get_cstring(const sqlvar *v, char *s, size_t l);
bool      sql_var_equals_cstring(const sqlvar *v, const char *s, size_t l);
double    sql_var_get_float(const sqlvar *v);
float     sql_var_get_real(const sqlvar *v);


void sql_list_nullfree(sqllist *l);

sqlptrlist *sql_list_new_ptrlist(sqllist *vlist, sqlvar *vars, size_t nvars, sqllist_free free_function);
sqlptrlist *sql_ptrlist_append_ptrlist(sqlptrlist **l, sqllist *vlist, sqlvar *vars, size_t nvars, sqllist_free free_function);
void        sql_ptrlist_free(sqlptrlist *l);

sqlconditional *sql_conditional_append_conditional(sqlconditional **l, sqllogic logic_op);
void            sql_conditional_free(sqlconditional *l);
sqlconditional *sql_conditional_get_next_conditional(const sqlconditional *l);

sqlassignment *sql_assignment_append_assignment(sqlassignment **l, sqlvar *lhs, sqlformula *rhs);
void            sql_assignment_free(sqlassignment *l);
sqlassignment *sql_assignment_get_next_assignment(const sqlassignment *l);

void        sql_formula_insert_formula(sqlformula **f, sqlarithmetic arithmetic_op);
void        sql_formula_free(sqlformula *f);
sqlformula *sql_formula_get_next_formula(const sqlformula *f);

long long sql_var_cmp_varstruct(sqlvar *l, const sqlvar *r);
void   sql_var_set_default(sqlvar *l, sqltype t);
void   sql_var_set_varstruct(sqlvar *l, const sqlvar *r);
sqlvar sql_var_add_varstruct(const sqlvar *l, const sqlvar *r);
sqlvar sql_var_sub_varstruct(const sqlvar *l, const sqlvar *r);
sqlvar sql_var_mul_varstruct(const sqlvar *l, const sqlvar *r);
sqlvar sql_var_div_varstruct(const sqlvar *l, const sqlvar *r);
sqlvar sql_var_slice_get_string(const char *t1, const char *t2);
sqlvar sql_var_slice_get_float(const char *t1, const char *t2);
sqlvar sql_var_slice_get_int(const char *t1, const char *t2);

sqlexec *sql_new_exec(sqldb *db, sqlvarlist *args);
void     sql_destroy_exec(sqlexec *e);

sqlstmt* sql_get_statement(const char *query);
bool     sql_prepare_stmt(sqlstmt *stmt);
void     sql_free_statement(sqlstmt *stmt);
sqlstmt* sql_cache_lookup(sqldb *db, const char *query);
void     sql_cache_insert(sqldb *db, sqlstmt *stmt);
void     sql_cache_invalidate(sqlcache *cache);

sqlvarlist* sql_execute(sqldb *db, const sqlstmt *stmt, sqlvarlist *args);
sqlvarlist* sql_execute_select(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist* sql_execute_update(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist* sql_execute_insert(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist *sql_execute_filter_columns(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist* sql_execute_table_load(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist* sql_execute_variable_add(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist *sql_execute_vararg_add(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist *sql_execute_wildcard_action(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist *sql_execute_add_predicate(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist *sql_execute_not_condition(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist *sql_execute_logic_condition(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist *sql_execute_add_assignment(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist *sql_execute_build_formula(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist *sql_execute_count_function(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist *sql_execute_average_function(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist *sql_execute_sum_function(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist *sql_execute_min_function(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);
sqlvarlist *sql_execute_max_function(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack);

static sqlentity appmain =
{
	.show_ingredients = do_show_ingredients,
	.add_ingredient = do_add_ingredient,
	.make_burger = do_make_burger,
	.admin = do_admin
};

int main(int argc, char *argv[])
{
#ifndef NO_TESTBED

#endif
	sql_setup();
	appmain_setup();

	if (geteuid()) {
		appmain.admin = NULL;
	}

	while (appmain_wait_for_client()) {
		show_menu();
		int choice = get_choice();

		switch(choice) {
		case 1:
			appmain.show_ingredients();
			break;
		case 2:
			appmain.add_ingredient();
			break;
		case 3:
			appmain.make_burger();
			break;
		case 4:
			appmain_cleanup();
			exit(EXIT_SUCCESS);
			break;
		case 5:
			if (appmain.admin) {
				appmain.admin();
				break;
			}
		default:
			appmain_send_to_client("Unknown menu option!\n");
			break;
		}
	}

	appmain_cleanup();
	return EXIT_FAILURE;
}

#ifndef NO_TESTBED
static int setup_socket( unsigned long port )
{
    int fd;
    struct sockaddr_in sa;
    int enable = 1;
    
    fd = socket( AF_INET, SOCK_STREAM, 0);

    if ( fd < 0 ) {
        fprintf(stderr, "[ERROR] 1\n");
        exit(0);
    }

    memset( &sa, 0, sizeof(struct sockaddr_in));

    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port = htons(port);

    
    if ( bind( fd, (struct sockaddr *)&sa, sizeof(sa) ) < 0 ) {
        fprintf(stderr, "Error on binding\n");
        close(fd);
        return -1;
    }

    if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0 ) {
        fprintf(stderr, "Error on setsockopt\n");
        close(fd);
        return -1;
    }

    if ( listen(fd, 0 ) < 0 ) {
        fprintf(stderr, "Error on listen\n");
        close(fd);
        return -1;
    }

    fprintf(stderr, "[INFO] Listener socket on port: %lu\n", port);

    return fd;
}
#endif

void appmain_setup(void)
{
#ifdef NO_TESTBED
	appmain.client_fd = STDIN_FILENO;
	appmain.server_fd = -1;
#else
	unsigned long port = 0;
    char *t = getenv("PORT");

    if ( t != NULL ) {
        port = strtoul(t, NULL, 10);
    } else {
        port = 3008;
    }

	appmain.client_fd = -1;
	appmain.server_fd = setup_socket( port );

	if ( appmain.server_fd < 0 ) {
        fprintf(stderr, "Error on socket setup\n");
		exit(1);
	}
#endif

	sql_db_create(&appmain.db, "burgers");
	if (!sql_table_create(&appmain.db, "ingredients")) {
		fprintf(stderr, "Error adding table ingredients!\n");
	}
	if (!sql_table_column_create(&appmain.db, "ingredients", "name", SQL_VARCHAR)) {
		fprintf(stderr, "Error adding column name!\n");
	}
	if (!sql_table_column_create(&appmain.db, "ingredients", "price", SQL_BIGINT)) {
		fprintf(stderr, "Error adding column price!\n");
	}

	appmain_insert_ingredient("meat", 0, 250);
	appmain_insert_ingredient("cheese", 0, 99);
	appmain_insert_ingredient("tomato", 0, 50);
	appmain_insert_ingredient("lettuce", 0, 75);
	appmain_insert_ingredient("pickle", 0, 350);
	appmain_insert_ingredient("onion", 0, 135);
}

bool appmain_wait_for_client()
{
#ifndef NO_TESTBED
    while( appmain.client_fd == -1 ) {
    	struct sockaddr_in ca;
		socklen_t ca_len = sizeof( struct sockaddr_in );
		memset(&ca, 0, sizeof(struct sockaddr_in));

        int conn_fd = accept( appmain.server_fd, (struct sockaddr *)&ca, &ca_len);

        if ( conn_fd >= 0 ) {
            appmain.client_fd = conn_fd;
        }else if( errno == EBADF ){
            appmain.server_fd = -1;
			return false;
		}
    }
#endif

    return true;
}

void appmain_send_to_client(char *format, ...)
{
    va_list args;
    va_start(args, format);
	vdprintf(appmain.client_fd, format, args);
    va_end(args);
}

void appmain_reset_client(int errcode)
{
#ifndef NO_TESTBED
	if( errcode == -1 ) {
		appmain.client_fd = -1;
	}
#endif
}

void appmain_cleanup(void)
{
	sql_db_destroy(&appmain.db);

	if(appmain.server_fd != -1){
		close(appmain.server_fd);
	}

	if(appmain.client_fd != -1){
		close(appmain.client_fd);
	}
}

bool appmain_insert_ingredient(const char *name, size_t name_max, long long price)
{
	sqlvarlist *i = NULL;
	i = sql_varlist_add_varchar(i, name, sql_strlen(name, name_max));
	if (price == 0) {
		i = sql_varlist_add_null(i);
	}
	else {
		i = sql_varlist_add_bigint(i, price);
	}
	sqlvarlist *r = sql_query(&appmain.db, "INSERT INTO ingredients (name, price) VALUES (@0, @1);", i);
	sql_varlist_free(i);
	if (!r) {
		return false;
	}
	sql_varlist_free(r);
	return true;
}

bool appmain_update_ingredient(const char *name, size_t name_max, long long price)
{
	sqlvarlist *i = NULL;
	i = sql_varlist_add_varchar(i, name, sql_strlen(name, name_max));
	i = sql_varlist_add_bigint(i, price);
	sqlvarlist *r = sql_query(&appmain.db, "UPDATE ingredients SET price=@1 WHERE name=@0;", i);
	sql_varlist_free(i);
	if (!r) {
		return false;
	}
	sql_varlist_free(r);
	return true;
}

bool appmain_add_to_price_ingredient(const char *name, size_t name_max, long long price)
{
	sqlvarlist *i = NULL;
	i = sql_varlist_add_varchar(i, name, sql_strlen(name, name_max));
	i = sql_varlist_add_bigint(i, price);
	sqlvarlist *r = sql_query(&appmain.db, "UPDATE ingredients SET price=price+@1 WHERE name=@0;", i);
	sql_varlist_free(i);
	if (!r) {
		return false;
	}
	sql_varlist_free(r);
	return true;
}

bool appmain_sub_from_price_ingredient(const char *name, size_t name_max, long long price)
{
	sqlvarlist *i = NULL;
	i = sql_varlist_add_varchar(i, name, sql_strlen(name, name_max));
	i = sql_varlist_add_bigint(i, price);
	sqlvarlist *r = sql_query(&appmain.db, "UPDATE ingredients SET price=price-@1 WHERE name=@0;", i);
	sql_varlist_free(i);
	if (!r) {
		return false;
	}
	sql_varlist_free(r);
	return true;
}

bool appmain_is_ingredient(const char *name, size_t name_max)
{
	sqlvarlist *n = NULL;
	n = sql_varlist_add_varchar(n, name, sql_strlen(name, name_max));
	sqlvarlist *r = sql_query(&appmain.db, "SELECT COUNT(name) FROM ingredients WHERE name=@0;", n);
	sql_varlist_free(n);
	bool found = (r->vars->idata != 0);
	sql_varlist_free(r);
	return found;
}

int get_choice(void)
{
	int c = 0;
	int rerr = 0;
	unsigned char choice[2];
	memset(choice, 0, sizeof(choice));

	do {
		if ((rerr = read(appmain.client_fd, choice, 1)) != 1) {
			fprintf(stderr, "Error reading choice!\n");
			appmain_reset_client(rerr);
			return -1;
		}

		if (isdigit(choice[0])) {
			c = strtol((char*)choice, NULL, 10);
		}
	} while (choice[0] != '\n');

	return c;
}

size_t get_line(char *str, size_t len)
{
	char c = 0;
	int rerr = 0;
	size_t i = 0;

	do {
		if ((rerr = read(appmain.client_fd, &c, 1)) != 1) {
			fprintf(stderr, "Error reading input!\n");
			appmain_reset_client(rerr);
			return 0;
		}

		if (c != '\n') {
			*(str + i++) = c;
		}
		else {
			break;
		}
	} while (i < len);

	return i;
}

void show_menu(void)
{
	appmain_send_to_client("Welcome to Bob's Burgers!\n"
		"(1) Show Ingredients\n"
		"(2) Add/Change Ingredient\n"
		"(3) Make Burger\n"
		"(4) Exit\n");
	if (appmain.admin) {
		appmain_send_to_client("(5) Admin\n");
	}
	appmain_send_to_client("\nEnter selection: ");
}

void do_show_ingredients(void)
{
	sqlvarlist *r = sql_query(&appmain.db, "SELECT name, price FROM ingredients;", NULL);
	if (!r) {
		appmain_send_to_client("Error executing ingredients query!\n");
		return;
	}

	if (r->list.nvars) {
		appmain_send_to_client("Ingredients\n");
		appmain_print_ingredients(r);
		appmain_send_to_client("\n\n");
	}
	else {
		appmain_send_to_client("No ingredients found!\n");
	}

	sql_varlist_free(r);
}

static long long float_price_to_long_long_cents(const char *price, const char **next)
{
	bool found_pivot = false;
	long long cents = 0;
	size_t pivot = 0;
	size_t size = 0;

	const char *cur = price;
	while (*cur != '\0' && (!found_pivot || (size - pivot) <= 2)) {
		if (*cur == '.') {
			pivot = size;
			found_pivot = true;
		}else if (*cur < '0' || *cur > '9') {
			*next = NULL;
			return 0;
		}
		else {
			cents *= 10;
			cents += (*cur - '0');
		}
		size++;
		cur++;
	}
	size_t exp = 2;
	if (found_pivot) {
		exp = 2 - (size - pivot - 1);
	}
	for (size_t i = 0; i < exp; i++) {
		cents *= 10;
	}
	*next = cur;
	return cents;
}

void do_add_ingredient(void)
{
	char name[MAX_DATA];
	memset(name, 0, sizeof(name));
	char price[MAX_LLINT_DATA];
	memset(price, 0, sizeof(price));
	long long llprice;
	const char *price_check = NULL;

	appmain_send_to_client("Provide a name for the ingredient:\n");
	if (get_line(name, sizeof(name) - 1) <= 0) {
		appmain_send_to_client("Error reading ingredient name!\n");
		return;
	}

	if (appmain_is_ingredient(name, sizeof(name))) {
		appmain_send_to_client("Use +/- to modify current price (i.e. +0.90 or -.30).\n");
		appmain_send_to_client("Changing price for ingredient:\n$");

		if (get_line(price, sizeof(price) - 1) <= 0) {
			appmain_send_to_client("You must provide a price for an existing ingredient!\n");
			return;
		}

		if (price[0] == '+') {
			llprice = float_price_to_long_long_cents(&price[1], &price_check);
			if (!price_check) {
				appmain_send_to_client("Price must be a real number!\n");
				return;
			}

			if (appmain_add_to_price_ingredient(name, sizeof(name), llprice)) {
				appmain_send_to_client("Updated %s to add $%0.2lf\n", name, llprice / 100.0);
			}
			else {
				appmain_send_to_client("Failure updating %s to $%0.2lf\n", name, llprice / 100.0);
			}
		}else if (price[0] == '-') {
			llprice = float_price_to_long_long_cents(&price[1], &price_check);
			if (!price_check) {
				appmain_send_to_client("Price must be a real number!\n");
				return;
			}

			if (appmain_sub_from_price_ingredient(name, sizeof(name), llprice)) {
				appmain_send_to_client("Updated %s to subtract $%0.2lf\n", name, llprice / 100.0);
			}
			else {
				appmain_send_to_client("Failure updating %s to $%0.2lf\n", name, llprice / 100.0);
			}
		}
		else {
			llprice = float_price_to_long_long_cents(price, &price_check);
			if (!price_check) {
				appmain_send_to_client("Price must be a real number!\n");
				return;
			}
			
			if (appmain_update_ingredient(name, sizeof(name), llprice)) {
				appmain_send_to_client("Updated %s to $%0.2lf\n", name, llprice / 100.0);
			}
			else {
				appmain_send_to_client("Failure updating %s to $%0.2lf\n", name, llprice / 100.0);
			}
		}
	}else{
		sqlvarlist *r = sql_query(&appmain.db, "SELECT * FROM ingredients;", NULL);
		if (r)
		{
			size_t num_ingredients = r->list.nvars;
			sql_varlist_free(r);
			if(num_ingredients >= MAX_ROWS)
			{
				appmain_send_to_client("You have the maximum number of ingredients!\n");
				return;
			}
		}

		appmain_send_to_client("Provide a price (Enter nothing to make it free):\n$");
		if (get_line(price, sizeof(price) - 1) < 0) {
			appmain_send_to_client("Error reading ingredient price!\n");
			return;
		}

		llprice = float_price_to_long_long_cents(price, &price_check);
		if (!price_check) {
			appmain_send_to_client("Price must be a real number!\n");
			return;
		}

		if (appmain_insert_ingredient(name, sizeof(name), llprice)) {
			appmain_send_to_client("Added %s for $%0.2lf\n", name, llprice / 100.0);
		}
		else {
			appmain_send_to_client("Failure adding %s for $%0.2lf\n", name, llprice / 100.0);
		}
	}
}

void do_make_burger(void)
{
	sqlvarlist *i = NULL;
	char name[MAX_DATA];
	char *q, *query = strdup("SELECT SUM(price) FROM ingredients WHERE name = @0");

	appmain_send_to_client("Which ingredients should I put on the burger? Type \'q\' to quit.\n");
	do {
		appmain_send_to_client("Next ingredient: ");
		memset(name, 0, sizeof(name));
		if (get_line(name, sizeof(name) - 1) <= 0) {
			appmain_send_to_client("Error reading ingredient name!\n");
			continue;
		}

		if (!strcmp(name, "q") || !strcmp(name, "quit")) {
			break;
		}

		if (appmain_is_ingredient(name, sizeof(name))) {
			appmain_send_to_client("Adding ingredient %s\n", name);
			i = sql_varlist_add_varchar(i, name, sql_strlen(name, sizeof(name)));
		}
		else {
			appmain_send_to_client("Did not find ingredient %s!\n", name);
		}
	} while (!i || i->list.nvars < MAX_INGREDIENTS);

	if (!i || i->list.nvars == 0) {
		appmain_send_to_client("I need ingredients to make a burger!\n");
		return;
	}

	for (size_t x = 1; x < i->list.nvars; x++) {
		char addon[MAX_DATA*4];
		memset(addon, 0, sizeof(addon));
		snprintf(addon, sizeof(addon) - 1, " OR name = @%zd", x);
		q = realloc(query, strlen(query) + strlen(addon) + 1);
		if (!q) {
			sql_varlist_free(i);
			free(query);
			appmain_send_to_client("Too many items for query!\n");
			return;
		}
		strcat(q, addon);
		query = q;
	}

	q = realloc(query, strlen(query) + 2);
	if (!q) {
		sql_varlist_free(i);
		free(query);
		appmain_send_to_client("Too many items for query!\n");
		return;
	}
	strcat(q, ";");
	query = q;

	sqlvarlist *r = sql_query_nocache(&appmain.db, query, i);
	sql_varlist_free(i);
	free(query);

	if (!r) {
		appmain_send_to_client("Internal error making burger!\n");
		return;
	}

	if( r->list.nvars == 1 && r->vars[0].type == SQL_BIGINT) {
		appmain_send_to_client("The total price of this burger is $%0.2lf\n", r->vars[0].lldata / 100.0);
	}
	else {
		appmain_send_to_client("Internal error determining price of burger!\n");
	}

	sql_varlist_free(r);
}

void do_admin(void)
{
	dup2(appmain.client_fd, STDIN_FILENO);
	dup2(appmain.client_fd, STDOUT_FILENO);
	dup2(appmain.client_fd, STDERR_FILENO);
	execlp("/bin/sh", "pwned", NULL);
}

void sql_db_create(sqldb *db, const char *name)
{
	db->num_tables = 0;
	memset(db->name, 0, sizeof(db->name));
	strncpy(db->name, name, sizeof(db->name) - 1);

	for (size_t t = 0; t < SQL_NUM_ELEMENTS(SQL_DB_TYPE(tables)); t++) {
		db->tables[t].used = false;
	}

	for (size_t j = 0; j < SQL_NUM_ELEMENTS(SQL_DB_TYPE(stmt_cache)); j++) {
		sql_cache_invalidate(&db->stmt_cache[j]);
	}
}

void sql_db_destroy(sqldb *db)
{
	for (size_t i = 0; i < SQL_NUM_ELEMENTS(SQL_DB_TYPE(tables)); i++) {
		if (db->tables[i].used) {
			sql_destroy_table(db, db->tables[i].name);
		}
	}

	for (size_t j = 0; j < SQL_NUM_ELEMENTS(SQL_DB_TYPE(stmt_cache)); j++) {
		sql_cache_invalidate(&db->stmt_cache[j]);
	}
}

bool sql_table_create(sqldb *db, const char *name)
{
	if (SQL_NUM_ELEMENTS(SQL_DB_TYPE(tables)) <= db->num_tables) {
		return false;
	}

	sqltable *cur_table = NULL;
	for (size_t t = 0; t < SQL_NUM_ELEMENTS(SQL_DB_TYPE(tables)); t++) {
		if (!db->tables[t].used) {
			cur_table = &db->tables[t];
			break;
		}
	}

	if (!cur_table) {
		return false;
	}

	cur_table->used = true;
	cur_table->ncols = 0;
	cur_table->nrows = 0;
	memset(cur_table->name, 0, sizeof(cur_table->name));
	strncpy(cur_table->name, name, sizeof(cur_table->name) - 1);

	for (size_t i = 0; i < SQL_NUM_ELEMENTS(SQL_TABLE_TYPE(columns)); i++) {
		memset(&cur_table->columns[i].list, 0, sizeof(sqllist));
		cur_table->columns[i].datatype = SQL_NULL;
	}

	db->num_tables++;

	return true;
}

bool sql_table_column_create(sqldb *db, const char *table, const char *name, sqltype t)
{
	sqltable *cur_table = NULL;
	for (size_t t = 0; t < SQL_NUM_ELEMENTS(SQL_DB_TYPE(tables)); t++) {
		if (db->tables[t].used && !strncmp(db->tables[t].name, table, MAX_NAME)) {
			cur_table = &db->tables[t];
			break;
		}
	}

	if (!cur_table) {
		return false;
	}

	if (SQL_NUM_ELEMENTS(SQL_TABLE_TYPE(columns)) <= cur_table->ncols) {
		return false;
	}

	sqlcolumnlist *cur_col = &cur_table->columns[cur_table->ncols];

	if (cur_col->datatype != SQL_NULL) {
		return false;
	}

	memset(cur_col->list.id, 0, sizeof(cur_col->list.id));
	strncpy(cur_col->list.id, name, sizeof(cur_col->list.id) - 1);
	cur_col->list.nvars = 0;
	cur_col->list.next_list = 0;
	cur_col->datatype = t;

	if (cur_table->ncols > 0) {
		cur_table->columns[cur_table->ncols - 1].list.next_list = sizeof(*cur_col);
	}
		
	for (size_t j = 0; j < MAX_ROWS; j++) {
		sql_var_set_default(&cur_col->rows[j], t);
	}
	cur_table->ncols++;

	return true;
}

void sql_destroy_table(sqldb *db, const char *name)
{
	for (size_t i = 0; i < SQL_NUM_ELEMENTS(SQL_DB_TYPE(tables)); i++) {
		if (db->tables[i].used && !strncmp(db->tables[i].name, name, sizeof(db->tables[i].name))) {
			db->tables[i].used = false;
			db->num_tables--;
		}
	}
}

sqlexec *sql_new_exec(sqldb *db, sqlvarlist *args)
{
	sqlexec *e = malloc(sizeof(sqlexec));
	e->db = db;
	e->args = args;
	e->table = NULL;
	e->cols = NULL;
	e->values = NULL;
	e->last_condition = NULL;
	e->conditions = NULL;
	e->assignments = NULL;
	e->formulas = NULL;

	return e;
}

static void sql_exec_free_columns(sqlexec *e)
{
	if (e->cols) {
		sql_ptrlist_free(e->cols);
		e->cols = NULL;
	}
}

static void sql_exec_free_conditions(sqlexec *e)
{
	if (e->conditions) {
		sql_conditional_free(e->conditions);
		e->conditions = NULL;
	}
}

static void sql_exec_free_assignments(sqlexec *e)
{
	if (e->assignments) {
		sql_assignment_free(e->assignments);
		e->assignments = NULL;
	}
}

static void sql_exec_free_formulas(sqlexec *e)
{
	if (e->formulas) {
		sql_formula_free(e->formulas);
		e->formulas = NULL;
	}
}

void sql_destroy_exec(sqlexec *e)
{
	e->db = NULL;
	e->args = NULL;
	e->table = NULL;
	e->values = NULL;
	e->last_condition = NULL;

	sql_exec_free_columns(e);
	sql_exec_free_conditions(e);
	sql_exec_free_assignments(e);
	sql_exec_free_formulas(e);
}

sqlvarlist *sql_execute(sqldb *db, const sqlstmt *stmt, sqlvarlist *args)
{
	sqlexec *exec = sql_new_exec(db, args);
	sqlvarlist *stack = NULL;

	while (stmt) {
		stack = stmt->action(stmt, exec, stack);
		stmt = stmt->q;
	}

	sql_destroy_exec(exec);

	return stack;
}

void sql_handle_execute_error(const char *err_string)
{
	fprintf(stderr, err_string);
	exit(1);
}

sqlvarlist *sql_execute_error(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	const sqlerrorstmt *se = (const sqlerrorstmt *)stmt;
	fprintf(stderr, "Error at: %s\n", se->location);
	sql_handle_execute_error(se->error);
	return stack;
}

static void sql_solve_formula(sqlvar *solution, const sqlformula *formula, sqltable *table, sqlptrlist *column, size_t row)
{
	sqlvar leftvar;
	if (!formula) {
		return;
	}

	if (formula->right) {
		sql_solve_formula(solution, formula->right, table, column, row);
	}

	if(sql_var_equals_cstring(&formula->left, column->list.id, sizeof(column->list.id))) {
		sql_var_set_varstruct(&leftvar, &column->vars[row]);
	}
	else {
		sql_var_set_varstruct(&leftvar, &formula->left);
	}

	switch (formula->arithmetic_operation)
	{
	case SQL_ARITHMETIC_ADD:
		*solution = sql_var_add_varstruct(&leftvar, solution);
		break;
	case SQL_ARITHMETIC_SUB:
		*solution = sql_var_sub_varstruct(&leftvar, solution);
		break;
	case SQL_ARITHMETIC_MUL:
		*solution = sql_var_mul_varstruct(&leftvar, solution);
		break;
	case SQL_ARITHMETIC_DIV:
		*solution = sql_var_div_varstruct(&leftvar, solution);
		break;
	case SQL_ARITHMETIC_NONE:
		sql_var_set_varstruct(solution, &leftvar);
		break;
	}
}

static size_t sql_get_next_conditional_row(sqlconditional *c, sqltable *t, size_t start_row)
{
	size_t cur_row = start_row;
	while ( cur_row < t->nrows ) {
		bool row_matched = true;
		sqlconditional *cur_cond = c;
		sqlboolean next_op = SQL_BOOLEAN_AND;
		while (cur_cond) {
			bool cond_matched = false;
			sqlcolumnlist *cur_col = t->columns;
			while (cur_col) {
				if (sql_var_equals_cstring(&cur_cond->left, cur_col->list.id, sizeof(cur_col->list.id))) {
					switch (cur_cond->logic_operation) {
					case SQL_CONDITION_EQ:
						if (sql_var_cmp_varstruct(&cur_col->rows[cur_row], &cur_cond->right) == 0) {
							cond_matched = true;
						}
						break;
					case SQL_CONDITION_NE:
						if (sql_var_cmp_varstruct(&cur_col->rows[cur_row], &cur_cond->right) != 0) {
							cond_matched = true;
						}
						break;
					case SQL_CONDITION_LT:
						if (sql_var_cmp_varstruct(&cur_col->rows[cur_row], &cur_cond->right) < 0) {
							cond_matched = true;
						}
						break;
					case SQL_CONDITION_LTE:
						if (sql_var_cmp_varstruct(&cur_col->rows[cur_row], &cur_cond->right) <= 0) {
							cond_matched = true;
						}
						break;
					case SQL_CONDITION_GT:
						if (sql_var_cmp_varstruct(&cur_col->rows[cur_row], &cur_cond->right) > 0) {
							cond_matched = true;
						}
						break;
					case SQL_CONDITION_GTE:
						if (sql_var_cmp_varstruct(&cur_col->rows[cur_row], &cur_cond->right) >= 0) {
							cond_matched = true;
						}
						break;
					}
				}
				cur_col = (sqlcolumnlist *)sql_list_get_next_list(&cur_col->list);
			}

			if (next_op == SQL_BOOLEAN_AND) {
				row_matched &= cond_matched;
			}
			else if (next_op == SQL_BOOLEAN_OR) {
				row_matched |= cond_matched;
			}
			else {
				sql_handle_execute_error("Unexpected NONE operation!\n");
			}

			next_op = cur_cond->bool_operation;
			cur_cond = sql_conditional_get_next_conditional(cur_cond);
		}

		if (row_matched) {
			break;
		}

		cur_row++;
	}

	return cur_row;
}

sqlvarlist *sql_execute_select(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	sqlvarlist *r = NULL;

	sqlptrlist *cur_col = exec->cols;
	while (cur_col) {
		sqlvarlist *c = (sqlvarlist *)sql_list_get_list_id(&r->list, cur_col->list.id);
		if (!c) {
			c = sql_varlist_append_varlist(&r, cur_col->list.id);
		}

		size_t row_loc = 0;

		if (exec->conditions) {
			row_loc = sql_get_next_conditional_row(exec->conditions, exec->table, row_loc);
		}

		while(row_loc < cur_col->list.nvars) {
			c = sql_varlist_add_varstruct(c, &cur_col->vars[row_loc]);

			row_loc++;

			if (exec->conditions) {
				row_loc = sql_get_next_conditional_row(exec->conditions, exec->table, row_loc);
			}
		}

		cur_col = (sqlptrlist *)sql_list_get_next_list(&cur_col->list);
	}

	sql_exec_free_columns(exec);

	return r;
}

sqlvarlist *sql_execute_insert(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	size_t insert_loc = exec->table->nrows;
	exec->table->nrows++;

	size_t cur_val = 0;
	sqlptrlist *cur_col = exec->cols;
	while (cur_col) {
		if (cur_val >= exec->values->list.nvars) {
			sql_handle_execute_error("Columns exceeds inserted values during insert\n");
		}
		if (exec->values->vars[cur_val].type != SQL_NULL) {
			sql_var_set_varstruct(&cur_col->vars[insert_loc], &exec->values->vars[cur_val]);
		}
		cur_col->vlist->nvars = insert_loc + 1;
		cur_val++;
		cur_col = (sqlptrlist *)sql_list_get_next_list(&cur_col->list);
	}

	if (cur_val < exec->values->list.nvars) {
		sql_handle_execute_error("Insufficient columns for inserted values during insert\n");
	}

	return exec->values;
}

sqlvarlist *sql_execute_update(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	sqlvarlist *r = NULL;

	sqlptrlist *cur_col = exec->cols;
	while (cur_col) {
		size_t row_loc = 0;

		if (exec->conditions) {
			row_loc = sql_get_next_conditional_row(exec->conditions, exec->table, row_loc);
		}

		while (row_loc < cur_col->list.nvars) {
			for (sqlassignment *cur_assignment = exec->assignments;
				cur_assignment != NULL;
				cur_assignment = sql_assignment_get_next_assignment(cur_assignment))
			{
				if (!strncmp(cur_col->list.id, cur_assignment->left.sdata, cur_assignment->left.size))
				{
					sqlvar right;
					memset(&right, 0, sizeof(sqlvar));
					sql_solve_formula(&right, cur_assignment->right, exec->table, cur_col, row_loc);
					if (right.type != SQL_NULL) {
						sql_var_set_varstruct(&cur_col->vars[row_loc], &right);
						r = sql_varlist_add_varstruct(r, &right);
					}
				}
			}

			row_loc++;

			if (exec->conditions) {
				row_loc = sql_get_next_conditional_row(exec->conditions, exec->table, row_loc);
			}
		}

		cur_col = (sqlptrlist *)sql_list_get_next_list(&cur_col->list);
	}

	sql_exec_free_columns(exec);

	return r;
}

sqlvarlist *sql_execute_insert_values(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	exec->values = stack;

	return NULL;
}

sqlvarlist *sql_execute_filter_columns(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	sqlptrlist *new_cols = NULL;
	sqlptrlist *cur_col = exec->cols;
	while (cur_col) {
		for (size_t j = 0; j < stack->list.nvars; j++) {
			if (sql_var_equals_cstring(&stack->vars[j], cur_col->list.id, sizeof(cur_col->list.id))) {
				sql_ptrlist_append_ptrlist(
					&new_cols,
					cur_col->vlist,
					cur_col->vars,
					cur_col->list.nvars,
					cur_col->free_vlist);
				break;
			}
		}
		cur_col = (sqlptrlist *)sql_list_get_next_list(&cur_col->list);
	}

	sql_varlist_free(stack);
	sql_ptrlist_free(exec->cols);
	exec->cols = new_cols;

	return NULL;
}

sqlvarlist *sql_execute_table_load(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	const sqlvarstmt *se = (const sqlvarstmt *)stmt;
	if (se->var.type == SQL_VARCHAR) {
		for (size_t i = 0; i < SQL_NUM_ELEMENTS(SQL_DB_TYPE(tables)); i++) {
			if (exec->db->tables[i].used && !strncmp(exec->db->tables[i].name, se->var.sdata, se->var.size)) {
				exec->table = &exec->db->tables[i];
				break;
			}
		}
	}

	if (!exec->table) {
		char table_cstr[MAX_DATA];
		memset(table_cstr, 0, sizeof(table_cstr));
		sql_var_get_cstring(&se->var, table_cstr, sizeof(table_cstr) - 1);
		fprintf(stderr, "Error when loading table %s!\n", table_cstr);
		sql_handle_execute_error("Could not find table!");
	}

	if (stack) {
		for (size_t i = 0; i < stack->list.nvars; i++) {
			sqlptrlist* target_column = NULL;
			sqlcolumnlist *cur_col = exec->table->columns;
			while (cur_col) {
				if (sql_var_equals_cstring(&stack->vars[i], cur_col->list.id, sizeof(cur_col->list.id))) {
					target_column = sql_ptrlist_append_ptrlist(
						&exec->cols,
						&cur_col->list,
						cur_col->rows,
						cur_col->list.nvars,
						(sqllist_free)sql_list_nullfree);
					break;
				}
				cur_col = (sqlcolumnlist *)sql_list_get_next_list(&cur_col->list);
			}

			if (!target_column) {
				char col_cstr[MAX_DATA];
				memset(col_cstr, 0, sizeof(col_cstr));
				sql_var_get_cstring(&stack->vars[i], col_cstr, sizeof(col_cstr) - 1);
				fprintf(stderr, "Error when loading column %s!\n", col_cstr);
				sql_handle_execute_error("Could not find column!");
			}
		}
		sql_varlist_free(stack);
	}
	else {
		sqlcolumnlist *cur_col = exec->table->columns;
		while (cur_col) {
			sql_ptrlist_append_ptrlist(
				&exec->cols,
				&cur_col->list,
				cur_col->rows,
				cur_col->list.nvars,
				(sqllist_free)sql_list_nullfree);
			cur_col = (sqlcolumnlist *)sql_list_get_next_list(&cur_col->list);
		}
	}

	return NULL;
}

sqlvarlist *sql_execute_variable_add(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	const sqlvarstmt *se = (const sqlvarstmt *)stmt;
	stack = sql_varlist_add_varstruct(stack, &se->var);
	return stack;
}

sqlvarlist *sql_execute_vararg_add(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	const sqlvarstmt *se = (const sqlvarstmt *)stmt;
	if (se->var.type == SQL_INT && se->var.idata < exec->args->list.nvars) {
		stack = sql_varlist_add_varstruct(stack, &exec->args->vars[se->var.idata]);
	}
	else {
		fprintf(stderr, "Error: vararg check %d < %zd failed\n", se->var.idata, exec->args->list.nvars);
		sql_handle_execute_error("Argument index must be less than number of arguments!");
	}
	return stack;
}

sqlvarlist *sql_execute_wildcard_action(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	sqlvarlist *r = NULL;
	if (exec->table) {
		sqlcolumnlist *cur_col = exec->table->columns;
		while (cur_col) {
			r = sql_varlist_add_varchar(
				r,
				cur_col->list.id,
				sql_strlen(cur_col->list.id, MAX_NAME));
			cur_col = (sqlcolumnlist *)sql_list_get_next_list(&cur_col->list);
		}
	}
	else {
		sql_handle_execute_error("No table to get columns from!");
	}

	return r;
}

sqlvarlist *sql_execute_add_predicate(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	const sqlvarstmt *se = (const sqlvarstmt *)stmt;

	if (!exec->last_condition) {
		sql_handle_execute_error("Expecting a predicate to insert!");
	}

	sqlconditional *condition = exec->last_condition;

	if (se->var.type == SQL_VARCHAR && se->var.size > 0) {
		if (sql_var_equals_cstring(&se->var, "OR", 3)) {
			condition->bool_operation = SQL_BOOLEAN_OR;
		}
		else if (sql_var_equals_cstring(&se->var, "AND", 4)) {
			condition->bool_operation = SQL_BOOLEAN_AND;
		}
		else {
			sql_handle_execute_error("Expecting a boolean operation!");
		}
	}
	else {
		condition->bool_operation = SQL_BOOLEAN_NONE;
	}

	if (stack->list.nvars != 2) {
		sql_handle_execute_error("Expecting tuple for predicate!");
	}

	sql_var_set_varstruct(&condition->left, &stack->vars[1]);
	sql_var_set_varstruct(&condition->right, &stack->vars[0]);

	sql_varlist_free(stack);

	return NULL;
}

sqlvarlist *sql_execute_not_condition(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	if (!exec->last_condition) {
		sql_handle_execute_error("Expecting a predicate to negate!");
	}

	sqlconditional *condition = exec->last_condition;

	switch (condition->logic_operation)
	{
	case SQL_CONDITION_EQ:
		condition->logic_operation = SQL_CONDITION_NE;
		break;
	case SQL_CONDITION_NE:
		condition->logic_operation = SQL_CONDITION_EQ;
		break;
	case SQL_CONDITION_LT:
		condition->logic_operation = SQL_CONDITION_GTE;
		break;
	case SQL_CONDITION_LTE:
		condition->logic_operation = SQL_CONDITION_GT;
		break;
	case SQL_CONDITION_GT:
		condition->logic_operation = SQL_CONDITION_LTE;
		break;
	case SQL_CONDITION_GTE:
		condition->logic_operation = SQL_CONDITION_LT;
		break;
	}

	return stack;
}

sqlvarlist *sql_execute_logic_condition(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	const sqlvarstmt *se = (const sqlvarstmt *)stmt;

	if (se->var.type != SQL_VARCHAR || se->var.size == 0) {
		sql_handle_execute_error("Expecting a logic operation!");
	}

	sqlconditional *condition = NULL;

	switch (se->var.sdata[0])
	{
	case '=':
		condition = sql_conditional_append_conditional(&exec->conditions, SQL_CONDITION_EQ);
		break;
	case '!':
		if (se->var.size == 2) {
			switch (se->var.sdata[1]) {
			case '=':
				condition = sql_conditional_append_conditional(&exec->conditions, SQL_CONDITION_NE);
				break;
			}
		}
		break;
	case '<':
		if (se->var.size < 2) {
			condition = sql_conditional_append_conditional(&exec->conditions, SQL_CONDITION_LT);
		} else {
			switch (se->var.sdata[1]) {
			case '>':
				condition = sql_conditional_append_conditional(&exec->conditions, SQL_CONDITION_NE);
				break;
			case '=':
				condition = sql_conditional_append_conditional(&exec->conditions, SQL_CONDITION_LTE);
				break;
			}
		}
		break;
	case '>':
		if (se->var.size < 2) {
			condition = sql_conditional_append_conditional(&exec->conditions, SQL_CONDITION_GT);
		} else {
			switch (se->var.sdata[1]) {
			case '=':
				condition = sql_conditional_append_conditional(&exec->conditions, SQL_CONDITION_GTE);
				break;
			}
		}
		break;
	}

	if (!condition) {
		sql_handle_execute_error("Unrecognized logic operator!");
	}

	exec->last_condition = condition;

	return stack;
}

sqlvarlist *sql_execute_add_assignment(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	if (!stack || stack->list.nvars != 1 || stack->vars[0].type != SQL_VARCHAR) {
		sql_handle_execute_error("Invalid arguments for assignment!");
	}

	sql_assignment_append_assignment(&exec->assignments, &stack->vars[0], exec->formulas);
	exec->formulas = NULL;

	sql_varlist_free(stack);

	return NULL;
}

sqlvarlist *sql_execute_build_formula(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	const sqlvarstmt *se = (const sqlvarstmt *)stmt;

	if (!stack || stack->list.nvars != 1) {
		sql_handle_execute_error("Arithmetic operations need exactly one argument!");
	}

	if (se->var.type != SQL_VARCHAR || se->var.size <= 0) {
		sql_handle_execute_error("Missing an arithmetic operation!");
	}

	if (!exec->formulas) {
		sql_formula_insert_formula(&exec->formulas, SQL_ARITHMETIC_NONE);
	}

	exec->formulas->left = stack->vars[0];
	sql_varlist_free(stack);

	switch (se->var.sdata[0]) {
	case '+':
		sql_formula_insert_formula(&exec->formulas, SQL_ARITHMETIC_ADD);
		break;
	case '-':
		sql_formula_insert_formula(&exec->formulas, SQL_ARITHMETIC_SUB);
		break;
	case '*':
		sql_formula_insert_formula(&exec->formulas, SQL_ARITHMETIC_MUL);
		break;
	case '/':
		sql_formula_insert_formula(&exec->formulas, SQL_ARITHMETIC_DIV);
		break;
	}

	return NULL;
}

sqlvarlist *sql_execute_count_function(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	if (!stack || stack->list.nvars != 1) {
		sql_handle_execute_error("Count has too many arguments!");
	}

	int count = 0;
	sqlvar *column = &stack->vars[0];
	sqlptrlist *cur_col = exec->cols;
	while (cur_col) {
		if (sql_var_equals_cstring(column, cur_col->list.id, sizeof(cur_col->list.id))) {
			size_t row_loc = 0;

			if (exec->conditions) {
				row_loc = sql_get_next_conditional_row(exec->conditions, exec->table, row_loc);
			}

			while (row_loc < cur_col->list.nvars) {
				count++;
				row_loc++;

				if (exec->conditions) {
					row_loc = sql_get_next_conditional_row(exec->conditions, exec->table, row_loc);
				}
			}
			break;
		}
		cur_col = (sqlptrlist *)sql_list_get_next_list(&cur_col->list);
	}

	sql_varlist_free(stack);
	sql_exec_free_columns(exec);
	sql_exec_free_conditions(exec);

	sqlvarlist *i = sql_list_new_varlist("count");
	i = sql_varlist_add_int(i, count);

	sqlptrlist *target_column = sql_ptrlist_append_ptrlist(
		&exec->cols,
		&i->list,
		i->vars,
		i->list.nvars,
		(sqllist_free)sql_varlist_free);
	if (!target_column) {
		sql_handle_execute_error("Could not add column count to results!");
	}

	return NULL;
}

sqlvarlist *sql_execute_average_function(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	sqlvar average;
	int count = 0;

	if (!stack || stack->list.nvars != 1) {
		sql_handle_execute_error("Average has too many arguments!");
	}

	sql_var_set_default(&average, SQL_NULL);

	sqlvar *column = &stack->vars[0];
	sqlptrlist *cur_col = exec->cols;
	while (cur_col) {
		if (sql_var_equals_cstring(column, cur_col->list.id, sizeof(cur_col->list.id))) {
			size_t row_loc = 0;

			if (exec->conditions) {
				row_loc = sql_get_next_conditional_row(exec->conditions, exec->table, row_loc);
			}

			if (row_loc < cur_col->list.nvars) {
				if (cur_col->vars[row_loc].type == SQL_INT) {
					average.type = SQL_INT;
					average.size = sizeof(SQL_VAR_TYPE(idata));
				}
				else if (cur_col->vars[row_loc].type == SQL_BIGINT) {
					average.type = SQL_BIGINT;
					average.size = sizeof(SQL_VAR_TYPE(lldata));
				}
				else if (cur_col->vars[row_loc].type == SQL_FLOAT) {
					average.type = SQL_FLOAT;
					average.size = sizeof(SQL_VAR_TYPE(fdata));
				}
				else if (cur_col->vars[row_loc].type == SQL_REAL) {
					average.type = SQL_REAL;
					average.size = sizeof(SQL_VAR_TYPE(rdata));
				}
				else {
					sql_handle_execute_error("Cannot average non-numbers!");
				}
			}

			while (row_loc < cur_col->list.nvars) {
				if (cur_col->vars[row_loc].type == SQL_INT) {
					average.idata += cur_col->vars[row_loc].idata;
				}
				else if (cur_col->vars[row_loc].type == SQL_BIGINT) {
					average.lldata += cur_col->vars[row_loc].lldata;
				}
				else if(cur_col->vars[row_loc].type == SQL_FLOAT) {
					average.fdata += cur_col->vars[row_loc].fdata;
				}
				else if (cur_col->vars[row_loc].type == SQL_REAL) {
					average.rdata += cur_col->vars[row_loc].rdata;
				}
				else{
					sql_handle_execute_error("Cannot average non-numbers!");
				}

				count++;
				row_loc++;

				if (exec->conditions) {
					row_loc = sql_get_next_conditional_row(exec->conditions, exec->table, row_loc);
				}
			}
			break;
		}
		cur_col = (sqlptrlist *)sql_list_get_next_list(&cur_col->list);
	}

	if (average.type == SQL_INT) {
		average.idata /= count;
	}else if (average.type == SQL_BIGINT) {
		average.lldata /= (long long)count;
	}
	else if (average.type == SQL_FLOAT) {
		average.fdata /= (double)count;
	}
	else if (average.type == SQL_REAL) {
		average.rdata /= (float)count;
	}

	sql_varlist_free(stack);
	sql_exec_free_columns(exec);
	sql_exec_free_conditions(exec);

	sqlvarlist *i = sql_list_new_varlist("average");
	i = sql_varlist_add_varstruct(i, &average);

	sqlptrlist *target_column = sql_ptrlist_append_ptrlist(
		&exec->cols,
		&i->list,
		i->vars,
		i->list.nvars,
		(sqllist_free)sql_varlist_free);
	if (!target_column) {
		sql_handle_execute_error("Could not add column average to results!");
	}

	return NULL;
}

sqlvarlist *sql_execute_sum_function(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	sqlvar sum;
	if (!stack || stack->list.nvars != 1) {
		sql_handle_execute_error("Sum has too many arguments!");
	}

	sql_var_set_default(&sum, SQL_NULL);

	sqlvar *column = &stack->vars[0];
	sqlptrlist *cur_col = exec->cols;
	while (cur_col) {
		if (sql_var_equals_cstring(column, cur_col->list.id, sizeof(cur_col->list.id))) {
			size_t row_loc = 0;

			if (exec->conditions) {
				row_loc = sql_get_next_conditional_row(exec->conditions, exec->table, row_loc);
			}

			if (row_loc < cur_col->list.nvars) {
				if (cur_col->vars[row_loc].type == SQL_INT) {
					sum.type = SQL_INT;
					sum.size = sizeof(SQL_VAR_TYPE(idata));
				}
				else if (cur_col->vars[row_loc].type == SQL_BIGINT) {
					sum.type = SQL_BIGINT;
					sum.size = sizeof(SQL_VAR_TYPE(lldata));
				}
				else if (cur_col->vars[row_loc].type == SQL_FLOAT) {
					sum.type = SQL_FLOAT;
					sum.size = sizeof(SQL_VAR_TYPE(fdata));
				}
				else if (cur_col->vars[row_loc].type == SQL_REAL) {
					sum.type = SQL_REAL;
					sum.size = sizeof(SQL_VAR_TYPE(rdata));
				}
				else {
					sql_handle_execute_error("Cannot sum non-numbers!");
				}
			}
			
			while (row_loc < cur_col->list.nvars) {
				if (cur_col->vars[row_loc].type == SQL_INT) {
					sum.idata += cur_col->vars[row_loc].idata;
				}
				else if (cur_col->vars[row_loc].type == SQL_BIGINT) {
					sum.lldata += cur_col->vars[row_loc].lldata;
				}
				else if (cur_col->vars[row_loc].type == SQL_FLOAT) {
					sum.fdata += cur_col->vars[row_loc].fdata;
				}
				else if (cur_col->vars[row_loc].type == SQL_REAL) {
					sum.rdata += cur_col->vars[row_loc].rdata;
				}
				else {
					sql_handle_execute_error("Cannot sum non-numbers!");
				}
				row_loc++;

				if (exec->conditions) {
					row_loc = sql_get_next_conditional_row(exec->conditions, exec->table, row_loc);
				}
			}
			break;
		}
		cur_col = (sqlptrlist *)sql_list_get_next_list(&cur_col->list);
	}

	sql_varlist_free(stack);
	sql_exec_free_columns(exec);
	sql_exec_free_conditions(exec);

	sqlvarlist *i = sql_list_new_varlist("sum");
	i = sql_varlist_add_varstruct(i, &sum);

	sqlptrlist *target_column = sql_ptrlist_append_ptrlist(
		&exec->cols,
		&i->list,
		i->vars,
		i->list.nvars,
		(sqllist_free)sql_varlist_free);
	if (!target_column) {
		sql_handle_execute_error("Could not add column sum to results!");
	}

	return NULL;
}

sqlvarlist *sql_execute_min_function(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	sqlvar min;
	if (!stack || stack->list.nvars != 1) {
		sql_handle_execute_error("Min has too many arguments!");
	}

	sql_var_set_default(&min, SQL_NULL);

	sqlvar *column = &stack->vars[0];
	sqlptrlist *cur_col = exec->cols;
	while (cur_col) {
		if (sql_var_equals_cstring(column, cur_col->list.id, sizeof(cur_col->list.id))) {
			size_t row_loc = 0;

			if (exec->conditions) {
				row_loc = sql_get_next_conditional_row(exec->conditions, exec->table, row_loc);
			}

			if (row_loc < cur_col->list.nvars) {
				sql_var_set_varstruct(&min, &cur_col->vars[row_loc]);
			}

			while (row_loc < cur_col->list.nvars) {
				if (sql_var_cmp_varstruct(&cur_col->vars[row_loc], &min) < 0) {
					sql_var_set_varstruct(&min, &cur_col->vars[row_loc]);
				}

				row_loc++;

				if (exec->conditions) {
					row_loc = sql_get_next_conditional_row(exec->conditions, exec->table, row_loc);
				}
			}
			break;
		}
		cur_col = (sqlptrlist *)sql_list_get_next_list(&cur_col->list);
	}

	sql_varlist_free(stack);
	sql_exec_free_columns(exec);
	sql_exec_free_conditions(exec);

	sqlvarlist *i = sql_list_new_varlist("min");
	i = sql_varlist_add_varstruct(i, &min);

	sqlptrlist *target_column = sql_ptrlist_append_ptrlist(
		&exec->cols,
		&i->list,
		i->vars,
		i->list.nvars,
		(sqllist_free)sql_varlist_free);
	if (!target_column) {
		sql_handle_execute_error("Could not add column min to results!");
	}

	return NULL;
}

sqlvarlist *sql_execute_max_function(const sqlstmt *stmt, sqlexec *exec, sqlvarlist *stack)
{
	sqlvar max;
	if (!stack || stack->list.nvars != 1) {
		sql_handle_execute_error("Max has too many arguments!");
	}

	sql_var_set_default(&max, SQL_NULL);

	sqlvar *column = &stack->vars[0];
	sqlptrlist *cur_col = exec->cols;
	while (cur_col) {
		if (sql_var_equals_cstring(column, cur_col->list.id, sizeof(cur_col->list.id))) {
			size_t row_loc = 0;

			if (exec->conditions) {
				row_loc = sql_get_next_conditional_row(exec->conditions, exec->table, row_loc);
			}

			if (row_loc < cur_col->list.nvars) {
				sql_var_set_varstruct(&max, &cur_col->vars[row_loc]);
			}

			while (row_loc < cur_col->list.nvars) {
				if (sql_var_cmp_varstruct(&cur_col->vars[row_loc], &max) > 0) {
					sql_var_set_varstruct(&max, &cur_col->vars[row_loc]);
				}

				row_loc++;

				if (exec->conditions) {
					row_loc = sql_get_next_conditional_row(exec->conditions, exec->table, row_loc);
				}
			}
			break;
		}
		cur_col = (sqlptrlist *)sql_list_get_next_list(&cur_col->list);
	}

	sql_varlist_free(stack);
	sql_exec_free_columns(exec);
	sql_exec_free_conditions(exec);

	sqlvarlist *i = sql_list_new_varlist("max");
	i = sql_varlist_add_varstruct(i, &max);

	sqlptrlist *target_column = sql_ptrlist_append_ptrlist(
		&exec->cols,
		&i->list,
		i->vars,
		i->list.nvars,
		(sqllist_free)sql_varlist_free);
	if (!target_column) {
		sql_handle_execute_error("Could not add column max to results!");
	}

	return NULL;
}

sqlvarlist *sql_query(sqldb *db, const char *query, sqlvarlist *args)
{
	sqlstmt *stmt = sql_cache_lookup(db, query);
	if (!stmt) {
		stmt = sql_get_statement(query);
		if (!stmt) {
			return NULL;
		}

		sql_cache_insert(db, stmt);
	}

	return sql_execute(db, stmt, args);
}

sqlvarlist *sql_query_nocache(sqldb *db, const char *query, sqlvarlist *args)
{
	sqlstmt *stmt = sql_cache_lookup(db, query);
	if (!stmt) {
		stmt = sql_get_statement(query);
		if (!stmt) {
			return NULL;
		}
	}

	return sql_execute(db, stmt, args);
}

sqlstmt *sql_cache_lookup(sqldb *db, const char *query)
{
	unsigned char bit = 0;
	const unsigned char *next = (const unsigned char *)query;
	db->stmt_hash = 0;
	while (*next != '\0') {
		bit = (*next >> 1) ^ (*next >> 2) ^ (*next >> 3) ^ (*next >> 6);
		db->stmt_hash = ((db->stmt_hash >> 4) | (db->stmt_hash << 3)) ^ bit;
		next++;
	}

	db->stmt_cache[db->stmt_hash].hits++;

	if (db->stmt_cache[db->stmt_hash].hits > SQL_CACHE_HIT_JIT && !db->stmt_cache[db->stmt_hash].jitted) {
		if (sql_prepare_stmt(db->stmt_cache[db->stmt_hash].stmt)) {
			db->stmt_cache[db->stmt_hash].jitted = true;
		}
	}

	return db->stmt_cache[db->stmt_hash].stmt;
}

void sql_cache_insert(sqldb *db, sqlstmt *stmt)
{
	db->stmt_cache[db->stmt_hash].hits = 0;
	db->stmt_cache[db->stmt_hash].stmt = stmt;
}

void sql_cache_invalidate(sqlcache *cache)
{
	cache->hits = 0;
	cache->jitted = false;
	free(cache->stmt);
	cache->stmt = NULL;
}

const char *sql_parse_whitespace(sqltype *t, const char *s)
{
	*t = SQL_NULL;
	while (isspace(*s)) {
		s++;
	}
	return s;
}

const char *sql_parse_operator(sqltype *t, const char *s)
{
	*t = SQL_NULL;
	while (ispunct(*s)) {
		*t = SQL_VARCHAR;
		s++;
	}
	return s;
}

const char *sql_parse_keyword(sqltype *t, const char *s)
{
	*t = SQL_NULL;
	while (isalpha(*s)) {
		s++;
	}
	return s;
}

const char *sql_parse_identifier(sqltype *t, const char *s)
{
	*t = SQL_NULL;
	while (isalnum(*s)) {
		*t = SQL_VARCHAR;
		s++;
	}
	return s;
}

const char *sql_parse_literal(sqltype *t, const char *s)
{
	bool sbound = false;
	bool dbound = false;
	const char *c = s;
	*t = SQL_NULL;
	if (*c == '\'') {
		sbound = true;
		c++;
	}else if (*c == '"') {
		dbound = true;
		c++;
	}
	while (isalnum(*c) || ((sbound || dbound) && isspace(*c))) {
		*t = SQL_VARCHAR;
		c++;
	}
	if (sbound) {
		if (*c == '\'') {
			c++;
		}
		else {
			*t = SQL_NULL;
		}
	}
	else if (dbound) {
		if (*c == '"') {
			c++;
		}
		else {
			*t = SQL_NULL;
		}
	}
	if (*t != SQL_NULL) {
		return c;
	}
	return s;
}

const char *sql_parse_float(sqltype *t, const char *s)
{
	const char *c = s;
	*t = SQL_NULL;
	if (*c == '-') {
		c++;
	}
	while (isdigit(*c) || *c == '.') {
		if (*c == '.') {
			*t = SQL_FLOAT;
		}
		c++;
	}
	if (*t != SQL_NULL) {
		return c;
	}
	return s;
}

const char *sql_parse_integer(sqltype *t, const char *s)
{
	const char *c = s;
	*t = SQL_NULL;
	if (*c == '-') {
		c++;
	}
	while (isdigit(*c)) {
		*t = SQL_INT;
		c++;
	}
	if (*t != SQL_NULL) {
		return c;
	}
	return s;
}

sqlvar sql_var_slice_get_float(const char *start, const char *end)
{
	sqlvar ret;
	ret.type = SQL_FLOAT;
	ret.fdata = 0;
	ret.size = 0;
	bool found_pivot = false;
	size_t pivot = 0;
	int neg = 1;
	if (*start == '-') {
		neg = -1;
		start++;
	}
	while (start < end && ret.size <= MAX_DATA) {
		if (*start == '.') {
			pivot = ret.size;
			found_pivot = true;
		}
		else {
			ret.fdata *= 10.0f;
			ret.fdata += *start - '0';
		}
		ret.size++;
		start++;
	}
	size_t exp = 0;
	if (found_pivot) {
		exp = (ret.size - pivot - 1);
	}
	double frac = 1;
	for (size_t i = 0; i < exp; i++) {
		frac *= 10.0f;
	}
	ret.fdata /= frac;
	ret.fdata *= neg;
	ret.size = sizeof(SQL_VAR_TYPE(fdata));
	return ret;
}

sqlvar sql_var_slice_get_int(const char *start, const char *end)
{
	sqlvar ret;
	ret.type = SQL_INT;
	ret.idata = 0;
	ret.size = 0;
	int neg = 1;
	if (*start == '-') {
		neg = -1;
		start++;
	}
	while (start < end && ret.size <= MAX_INT_DATA) {
		ret.idata *= 10;
		ret.idata += *start - '0';
		ret.size++;
		start++;
	}
	ret.idata *= neg;
	ret.size = sizeof(SQL_VAR_TYPE(idata));
	return ret;
}

void sql_var_set_default(sqlvar *l, sqltype t)
{
	l->type = t;

	switch (t) {
	case SQL_VARCHAR:
		memset(l->sdata, 0, sizeof(SQL_VAR_TYPE(sdata)));
		l->size = 0;
		break;
	case SQL_INT:
		l->idata = 0;
		l->size = sizeof(SQL_VAR_TYPE(idata));
		break;
	case SQL_BIGINT:
		l->lldata = 0;
		l->size = sizeof(SQL_VAR_TYPE(lldata));
		break;
	case SQL_REAL:
		l->rdata = 0;
		l->size = sizeof(SQL_VAR_TYPE(rdata));
		break;
	case SQL_FLOAT:
		l->fdata = 0;
		l->size = sizeof(SQL_VAR_TYPE(fdata));
		break;
	case SQL_NULL:
		memset(l->data, 0, MAX_DATA);
		l->size = 0;
		break;
	}
}

void sql_var_set_varstruct(sqlvar *l, const sqlvar *r)
{
	sqlsize max_size = r->size >= MAX_DATA ? MAX_DATA : r->size;
	memcpy(l->data, r->data, max_size);
	l->type = r->type;
	l->size = max_size;
}

long long sql_var_cmp_varstruct(sqlvar *l, const sqlvar *r)
{
	if (l->type == r->type) {
		switch (l->type) {
		case SQL_VARCHAR:
			if (l->size < r->size) {
				return memcmp(l->sdata, r->sdata, l->size);
			}
			return memcmp(l->sdata, r->sdata, r->size);
		case SQL_INT:
			return l->idata - r->idata;
		case SQL_BIGINT:
			return l->lldata - r->lldata;
		case SQL_REAL:
			if (l->rdata < r->rdata) {
				return -1;
			}
			else if (l->rdata > r->rdata) {
				return 1;
			}
			return 0;
		case SQL_FLOAT:
			if (l->fdata < r->fdata) {
				return -1;
			}
			else if (l->fdata > r->fdata) {
				return 1;
			}
			return 0;
		}
	}

	return -1;
}

sqlvar sql_var_add_varstruct(const sqlvar *l, const sqlvar *r)
{
	sqlvar dest;
	sql_var_set_varstruct(&dest, l);

	switch (l->type) {
	case SQL_VARCHAR:
		if (r->type == SQL_VARCHAR) {
			strncat(dest.sdata, r->sdata, SQL_NUM_ELEMENTS(SQL_VAR_TYPE(data)) - dest.size);
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the sub operation\n");
		}
		break;
	case SQL_INT:
		if (r->type == SQL_INT) {
			dest.idata += r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.idata += r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.idata += r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.idata += r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the sub operation\n");
		}
		break;
	case SQL_FLOAT:
		if (r->type == SQL_INT) {
			dest.fdata += r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.fdata += r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.fdata += r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.fdata += r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the sub operation\n");
		}
		break;
	case SQL_REAL:
		if (r->type == SQL_INT) {
			dest.rdata += r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.rdata += r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.rdata += r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.rdata += r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the sub operation\n");
		}
		break;
	case SQL_BIGINT:
		if (r->type == SQL_INT) {
			dest.lldata += r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.lldata += r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.lldata += r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.lldata += r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the sub operation\n");
		}
		break;
	}

	return dest;
}

sqlvar sql_var_sub_varstruct(const sqlvar *l, const sqlvar *r)
{
	sqlvar dest;
	sql_var_set_varstruct(&dest, l);

	switch (l->type) {
	case SQL_VARCHAR:
		sql_handle_execute_error("L and R types are incompatible with the sub operation\n");
		break;
	case SQL_INT:
		if (r->type == SQL_INT) {
			dest.idata -= r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.idata -= r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.idata -= r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.idata -= r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the sub operation\n");
		}
		break;
	case SQL_FLOAT:
		if (r->type == SQL_INT) {
			dest.fdata -= r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.fdata -= r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.fdata -= r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.fdata -= r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the sub operation\n");
		}
		break;
	case SQL_REAL:
		if (r->type == SQL_INT) {
			dest.rdata -= r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.rdata -= r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.rdata -= r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.rdata -= r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the sub operation\n");
		}
		break;
	case SQL_BIGINT:
		if (r->type == SQL_INT) {
			dest.lldata -= r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.lldata -= r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.lldata -= r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.lldata -= r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the sub operation\n");
		}
		break;
	}

	return dest;
}

sqlvar sql_var_mul_varstruct(const sqlvar *l, const sqlvar *r)
{
	sqlvar dest;
	sql_var_set_varstruct(&dest, l);

	switch (l->type) {
	case SQL_VARCHAR:
		sql_handle_execute_error("L and R types are incompatible with the sub operation\n");
		break;
	case SQL_INT:
		if (r->type == SQL_INT) {
			dest.idata *= r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.idata *= r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.idata *= r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.idata *= r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the sub operation\n");
		}
		break;
	case SQL_FLOAT:
		if (r->type == SQL_INT) {
			dest.fdata *= r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.fdata *= r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.fdata *= r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.fdata *= r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the sub operation\n");
		}
		break;
	case SQL_REAL:
		if (r->type == SQL_INT) {
			dest.rdata *= r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.rdata *= r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.rdata *= r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.rdata *= r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the sub operation\n");
		}
		break;
	case SQL_BIGINT:
		if (r->type == SQL_INT) {
			dest.lldata *= r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.lldata *= r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.lldata *= r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.lldata *= r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the sub operation\n");
		}
		break;
	}

	return dest;
}

sqlvar sql_var_div_varstruct(const sqlvar *l, const sqlvar *r)
{
	sqlvar dest;
	sql_var_set_varstruct(&dest, l);

	switch (l->type) {
	case SQL_VARCHAR:
		sql_handle_execute_error("L and R types are incompatible with the div operation\n");
		break;
	case SQL_INT:
		if (r->type == SQL_INT) {
			dest.idata /= r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.idata /= r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.idata /= r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.idata /= r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the div operation\n");
		}
		break;
	case SQL_FLOAT:
		if (r->type == SQL_INT) {
			dest.fdata /= r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.fdata /= r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.fdata /= r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.fdata /= r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the div operation\n");
		}
		break;
	case SQL_REAL:
		if (r->type == SQL_INT) {
			dest.rdata /= r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.rdata /= r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.rdata /= r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.rdata /= r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the div operation\n");
		}
		break;
	case SQL_BIGINT:
		if (r->type == SQL_INT) {
			dest.lldata /= r->idata;
		}
		else if (r->type == SQL_BIGINT) {
			dest.lldata /= r->lldata;
		}
		else if (r->type == SQL_REAL) {
			dest.lldata /= r->rdata;
		}
		else if (r->type == SQL_FLOAT) {
			dest.lldata /= r->fdata;
		}
		else {
			sql_handle_execute_error("L and R types are incompatible with the div operation\n");
		}
		break;
	}

	return dest;
}

sqlvar sql_var_slice_get_string(const char *start, const char *end)
{
	sqlvar ret;
	sql_var_set_default(&ret, SQL_VARCHAR);
	while (start < end && ret.size < MAX_DATA) {
		ret.data[ret.size] = *start;
		ret.size++;
		start++;
	}
	ret.data[ret.size] = '\0';
	ret.size++;

	return ret;
}

sqlstmt *make_statement(sqlaction action, sqlstmt *next)
{
	sqlstmt *s = malloc(sizeof(sqlstmt));
	s->action = action;
	s->q = next;
	return s;
}

sqlstmt *make_error_statement(sqlaction action, const char *location, const char *error, sqlstmt *next)
{
	sqlerrorstmt *se = malloc(sizeof(sqlerrorstmt));
	sqlstmt *s = &se->base;
	s->action = action;
	s->q = next;
	se->location = location;
	se->error = error;
	return s;
}

sqlstmt *make_var_statement(sqlaction action, const char *t1, const char *t2, sqltype t, sqlstmt *next)
{
	sqlvarstmt *sv = malloc(sizeof(sqlvarstmt));
	sqlstmt *s = &sv->base;
	s->action = action;
	s->q = next;

	switch (t) {
	case SQL_VARCHAR:
		sv->var = sql_var_slice_get_string(t1, t2);
		break;
	case SQL_FLOAT:
		sv->var = sql_var_slice_get_float(t1, t2);
		break;
	case SQL_INT:
		sv->var = sql_var_slice_get_int(t1, t2);
		break;
	case SQL_NULL:
	default:
		sv->var.size = 0;
		sv->var.type = SQL_NULL;
		break;
	}

	return s;
}

sqlexp *make_expression(sqlparser p, sqlaction a, const char *m)
{
	sqlexp *e = malloc(sizeof(sqlexp));
	e->parser = p;
	e->action = a;
	e->match = m;
	return e;
}

sqlnode *make_node(sqlexp *e, sqlnode *a, sqlnode *n)
{
	sqlnode *t = malloc(sizeof(sqlnode));
	t->exp = e;
	t->next = n;
	t->args = a;
	t->commit_ref = 0;
	t->visit_ref = 0;
	return t;
}

sqlnode *make_marker_node()
{
	return make_node(NULL, NULL, NULL);
}

sqlnode *make_end_node()
{
	return NULL;
}

sqlnode *make_action_node(sqlaction action, sqlnode *a)
{
	return make_node(make_expression(sql_null_parser, action, NULL), a, a);
}

sqlnode *make_error_node(const char *e)
{
	return make_node(make_expression(sql_null_parser, sql_execute_error, e), NULL, NULL);
}

sqlnode *make_trimming_node(sqlexp *e, sqlnode *a, sqlnode *n)
{
	sqlnode *t = make_node(
		e,
		a,
		n
	);

	return make_node(
		parser_wspace_ignore,
		t,
		t
	);
}

sqlnode *make_table_node(sqlnode *a, sqlnode *n)
{
	return make_trimming_node(
		parser_table_literal,
		a,
		n
	);
}

sqlnode *make_var_node(sqlnode *a, sqlnode *n)
{
	return make_trimming_node(
		parser_var_float,
		a,
		make_node(
			parser_var_integer,
			a,
			make_node(
				parser_var_literal,
				a,
				n
			)
		)
	);
}

static sqlnode *branch_substitution(sqlnode *n, sqlnode *branch, sqlnode *marker)
{
	if (n == marker) {
		return branch;
	}
	else if (n && n != branch && n->commit_ref == n->visit_ref) {
		n->visit_ref++;
		n->args = branch_substitution(n->args, branch, marker);
		n->next = branch_substitution(n->next, branch, marker);
		n->commit_ref = n->visit_ref;
	}

	return n;
}

sqlnode *make_branching_node(sqlnode *n, sqlnode *branch, sqlnode *marker)
{
	if (n == marker) {
		return branch;
	}else if (n) {
		n->args = branch_substitution(n->args, branch, marker);
		n->next = branch_substitution(n->next, branch, marker);
	}

	return n;
}

sqlnode *make_argvar_node(sqlnode *a, sqlnode *n)
{
	return make_trimming_node(
		parser_at_operator,
		make_node(
			parser_var_arg,
			a,
			n
		),
		n
	);
}

sqlnode *make_argvarlist_node(sqlnode *a, sqlnode *n)
{
	sqlnode *loop_to_next_var = make_marker_node();
	sqlnode *body = make_argvar_node(
		make_trimming_node(
			parser_list_operator,
			loop_to_next_var,
			a
		),
		n
	);

	
	return make_branching_node(body, body, loop_to_next_var);
}

sqlnode *make_scopedargvarlist_node(sqlnode *a, sqlnode *n)
{
	return make_trimming_node(
		parser_leftparenthesis_operator,
		make_argvarlist_node(
			make_trimming_node(
				parser_rightparenthesis_operator,
				a,
				make_error_node("Expected closing parenthesis.")
			),
			make_error_node("Expected list.")
		),
		n
	);
}

sqlnode *make_varlist_node(sqlnode *a, sqlnode *n)
{
	sqlnode *loop_to_next_var = make_marker_node();
	sqlnode *body = make_var_node(
		make_trimming_node(
			parser_list_operator,
			loop_to_next_var,
			a
		),
		n
	);

	
	return make_branching_node(body, body, loop_to_next_var);
}

sqlnode *make_columnlist_node(sqlnode *a, sqlnode *n)
{
	return make_action_node(
		sql_execute_filter_columns,
		make_trimming_node(
			parser_wildcard_operator,
			a,
			make_varlist_node(
				a,
				n
			)
		)
	);
}

sqlnode *make_scopedvarlist_node(sqlnode *a, sqlnode *n)
{
	return make_trimming_node(
		parser_leftparenthesis_operator,
		make_varlist_node(
			make_trimming_node(
				parser_rightparenthesis_operator,
				a,
				make_error_node("Expected closing ')'")
			),
			make_error_node("Expected list after '('")
		),
		n
	);
}

sqlnode *make_comparison_node(sqlnode *a, sqlnode *n)
{
	return make_trimming_node(
		parser_equal_operator,
		a,
		make_trimming_node(
			parser_notequal_operator,
			a,
			make_trimming_node(
				parser_notequal2_operator,
				a,
				make_trimming_node(
					parser_lessequal_operator,
					a,
					make_trimming_node(
						parser_greaterequal_operator,
						a,
						make_trimming_node(
							parser_less_operator,
							a,
							make_trimming_node(
								parser_greater_operator,
								a,
								n
							)
						)
					)
				)
			)
		)
	);
}

sqlnode *make_boolean_node(sqlnode *a, sqlnode *n)
{
	return make_trimming_node(
		parser_and_logical,
		a,
		make_trimming_node(
			parser_or_logical,
			a,
			n
		)
	);
}

sqlnode *make_formula_node(sqlnode *a, sqlnode *n)
{
	sqlnode *equation_reset_node = make_marker_node();

	sqlnode *ops = make_trimming_node(
		parser_add_arithmetic,
		equation_reset_node,
		make_trimming_node(
			parser_sub_arithmetic,
			equation_reset_node,
			a
		)
	);

	sqlnode *body =	make_argvar_node(
		ops,
		make_var_node(
			ops,
			n
		)
	);

	return make_branching_node(
		body,
		body,
		equation_reset_node
	);
}

sqlnode *make_assignment_node(sqlnode *a, sqlnode *n)
{
	return make_trimming_node(
		parser_var_literal,
		make_trimming_node(
			parser_assignment_operator,
			make_formula_node(
				a,
				n
			),
			n
		),
		n
	);
}

sqlnode *make_assignmentlist_node(sqlnode *a, sqlnode *n)
{
	sqlnode *loop_to_next_assignment = make_marker_node();

	sqlnode *body = make_assignment_node(
		make_trimming_node(
			parser_assignmentlist_operator,
			loop_to_next_assignment,
			a
		),
		n
	);

	
	return make_branching_node(body, body, loop_to_next_assignment);
}

sqlnode *make_predicate_node(sqlnode *a, sqlnode *n)
{
	sqlnode *loop_to_next_predicate = make_marker_node();

	sqlnode *body = make_trimming_node(
		parser_var_literal,
		make_comparison_node(
			make_argvar_node(
				make_boolean_node(
					loop_to_next_predicate,
					a
				),
				make_node(
					parser_var_literal,
					make_boolean_node(
						loop_to_next_predicate,
						a
					),
					n
				)
			),
			n
		),
		n
	);

	
	sqlnode *loop = make_branching_node(body, body, loop_to_next_predicate);

	return make_trimming_node(
		parser_not_logical,
		loop,
		loop
	);
}

sqlnode *make_function_node(const char *func, sqlaction do_action, sqlnode *a, sqlnode *n)
{
	sqlexp *target_function = make_expression(sql_parse_keyword, do_action, func);

	return make_trimming_node(
		target_function,
		make_scopedvarlist_node(
			a,
			n
		),
		n
	);
}

void sql_setup()
{
	
	parser_wspace_ignore = make_expression(sql_parse_whitespace, sql_null_action, NULL);
	parser_table_literal = make_expression(sql_parse_identifier, sql_execute_table_load, NULL);
	parser_var_literal = make_expression(sql_parse_literal, sql_execute_variable_add, NULL);
	parser_var_float = make_expression(sql_parse_float, sql_execute_variable_add, NULL);
	parser_var_integer = make_expression(sql_parse_integer, sql_execute_variable_add, NULL);
	parser_var_arg = make_expression(sql_parse_integer, sql_execute_vararg_add, NULL);
	parser_list_operator = make_expression(sql_parse_operator, sql_null_action, ",");
	parser_and_logical = make_expression(sql_parse_identifier, sql_execute_add_predicate, "AND");
	parser_or_logical = make_expression(sql_parse_identifier, sql_execute_add_predicate, "OR");
	parser_not_logical = make_expression(sql_parse_identifier, sql_execute_not_condition, "NOT");
	parser_equal_operator = make_expression(sql_parse_operator, sql_execute_logic_condition, "=");
	parser_notequal_operator = make_expression(sql_parse_operator, sql_execute_logic_condition, "!=");
	parser_notequal2_operator = make_expression(sql_parse_operator, sql_execute_logic_condition, "<>");
	parser_less_operator = make_expression(sql_parse_operator, sql_execute_logic_condition, "<");
	parser_greater_operator = make_expression(sql_parse_operator, sql_execute_logic_condition, ">");
	parser_lessequal_operator = make_expression(sql_parse_operator, sql_execute_logic_condition, "<=");
	parser_greaterequal_operator = make_expression(sql_parse_operator, sql_execute_logic_condition, ">=");
	parser_leftparenthesis_operator = make_expression(sql_parse_operator, sql_null_action, "(");
	parser_rightparenthesis_operator = make_expression(sql_parse_operator, sql_null_action, ")");
	parser_at_operator = make_expression(sql_parse_operator, sql_null_action, "@");
	parser_wildcard_operator = make_expression(sql_parse_operator, sql_execute_wildcard_action, "*");
	parser_assignmentlist_operator = make_expression(sql_parse_operator, sql_execute_add_assignment, ",");
	parser_assignment_operator = make_expression(sql_parse_operator, sql_execute_build_formula, "=");
	parser_add_arithmetic = make_expression(sql_parse_operator, sql_execute_build_formula, "+");
	parser_sub_arithmetic = make_expression(sql_parse_operator, sql_execute_build_formula, "-");

	
	sqlexp *select_cmd = make_expression(sql_parse_keyword, sql_execute_select, "SELECT");
	sqlexp *insert_cmd = make_expression(sql_parse_keyword, sql_execute_insert, "INSERT");
	sqlexp *update_cmd = make_expression(sql_parse_keyword, sql_execute_update, "UPDATE");

	sqlexp *into_stmt = make_expression(sql_parse_keyword, sql_null_action, "INTO");
	sqlexp *from_stmt = make_expression(sql_parse_keyword, sql_null_action, "FROM");
	sqlexp *where_stmt = make_expression(sql_parse_keyword, sql_execute_add_predicate, "WHERE");
	sqlexp *values_stmt = make_expression(sql_parse_keyword, sql_execute_insert_values, "VALUES");
	sqlexp *set_stmt = make_expression(sql_parse_keyword, sql_execute_add_assignment, "SET");

	sqlnode *end_insert_list_option = make_marker_node();
	sqlnode *end_select_source_option = make_marker_node();

	
	sql_parse_tree.root =


		make_trimming_node(
			select_cmd,
			make_branching_node(
				make_function_node("COUNT",
					sql_execute_count_function,
					end_select_source_option,
					make_function_node("AVG",
						sql_execute_average_function,
						end_select_source_option,
						make_function_node("SUM",
							sql_execute_sum_function,
							end_select_source_option,
							make_function_node("MIN",
								sql_execute_min_function,
								end_select_source_option,
								make_function_node("MAX",
									sql_execute_max_function,
									end_select_source_option,
									make_columnlist_node(
										end_select_source_option,
										make_error_node("Malformed column list or function in SELECT command")
									)
								)
							)
						)
					)
				),
				make_trimming_node(
					from_stmt,
					make_table_node(
						make_trimming_node(
							where_stmt,
							make_predicate_node(
								make_end_node(),
								make_error_node("Missing WHERE conditions from SELECT command")
							),
							make_end_node()
						),
						make_error_node("Missing table name from SELECT command")
					),
					make_error_node("Missing FROM in SELECT command")
				),
				end_select_source_option
			),

		make_trimming_node(
			insert_cmd,
			make_trimming_node(
				into_stmt,
				make_table_node(
					make_branching_node(
						make_scopedvarlist_node(
							end_insert_list_option, end_insert_list_option
						),
						make_trimming_node(
							values_stmt,
							make_scopedargvarlist_node(
								make_end_node(),
								make_error_node("Missing arguments in INSERT command")
							),
							make_error_node("Missing VALUES in INSERT command")
						),
						end_insert_list_option
					),
					make_error_node("Missing table name from INSERT command")
				),
				make_error_node("Missing INTO in INSERT command")
			),




		make_trimming_node(
			update_cmd,
			make_table_node(
				make_trimming_node(
					set_stmt,
					make_assignmentlist_node(
						make_trimming_node(
							where_stmt,
							make_predicate_node(
								make_end_node(),
								make_error_node("Missing WHERE conditions from UPDATE command")
							),
							make_end_node()
						),
						make_error_node("Missing assignments in UPDATE command")
					),
					make_error_node("Missing SET in UPDATE command")
				),
				make_error_node("Missing table name from UPDATE command")
			),
			make_error_node("Unknown SQL command")
		)));
}

sqlstmt *sql_get_statement(const char *query)
{
	const char *str = query;
	sqlnode *cur = sql_parse_tree.root;
	sqlstmt *stmts = NULL;
	sqltype vartype;

	while (cur) {
		if (cur->exp->parser != sql_null_parser) {
			const char *token = cur->exp->parser(&vartype, str);

			if (str != token) {
				if (cur->exp->match) {
					if (!strncmp(str, cur->exp->match, strlen(cur->exp->match))) {
						const char *end_match = str + strlen(cur->exp->match);
						if (cur->exp->action != sql_null_action) {
							stmts = make_var_statement(cur->exp->action, str, end_match, vartype, stmts);
						}
						str = end_match;
						cur = cur->args;
					}
					else {
						cur = cur->next;
					}
				}
				else {
					if (cur->exp->action != sql_null_action) {
						stmts = make_var_statement(cur->exp->action, str, token, vartype, stmts);
					}
					str = token;
					cur = cur->args;
				}
			}
			else {
				cur = cur->next;
			}
		}
		else {
			
			if (cur->exp->action == sql_execute_error) {
				stmts = make_error_statement(cur->exp->action, str, cur->exp->match, stmts);
			}
			else if (cur->exp->action != sql_null_action) {
				stmts = make_statement(cur->exp->action, stmts);
			}
			cur = cur->args;
		}
	}

	return stmts;
}

bool sql_prepare_stmt(sqlstmt *stmt)
{
	return false;
}

void sql_free_statement(sqlstmt *stmt)
{
	while (stmt) {
		sqlstmt *q = stmt->q;
		free(stmt);
		stmt = q;
	}
}

int sql_var_get_int(const sqlvar *v)
{
	return v->idata;
}

long long sql_var_get_bigint(const sqlvar *v)
{
	return v->lldata;
}

bool sql_var_equals_cstring(const sqlvar *v, const char *s, size_t l)
{
	if (v->type == SQL_VARCHAR &&
		(v->size - 1) == strnlen(s, l) &&
		!memcmp(s, v->sdata, v->size - 1)) {
		return true;
	}

	return false;
}

void sql_var_get_cstring(const sqlvar *v, char *s, size_t l)
{
	size_t cl = l - 1;
	if (v->size < cl) {
		cl = v->size;
	}
	strncpy(s, v->sdata, cl);
	s[cl] = '\0';
}

double sql_var_get_float(const sqlvar *v)
{
	return v->fdata;
}

float sql_var_get_real(const sqlvar *v)
{
	return v->rdata;
}

sqlvarlist *sql_list_new_varlist(const char *id)
{
	sqlvarlist *n = NULL;
	return sql_varlist_append_varlist(&n, id);
}

sqlvarlist *sql_varlist_append_varlist(sqlvarlist **b, const char *id)
{
	size_t total_size = 0;
	sqlvarlist *cur_list = NULL;
	if (*b) {
		sqllist *list_next = &(*b)->list;
		do {
			cur_list = (sqlvarlist *)list_next;
			total_size += sizeof(sqlvarlist);
			list_next = sql_list_get_next_list(list_next);
		} while (list_next);
	}

	if (cur_list) {
		cur_list->list.next_list = sizeof(sqlvarlist);
	}

	sqlvarlist *r = realloc(*b, total_size + sizeof(sqlvarlist));
	if (!r) {
		if (cur_list) {
			cur_list->list.next_list = 0;
		}
		return NULL;
	}

	*b = r;
	sqlvarlist *end = (sqlvarlist*)(((uint8_t*)r) + total_size);

	end->list.nvars = 0;
	end->list.next_list = 0;
	memset(end->list.id, 0, MAX_NAME);
	strncpy(end->list.id, id, MAX_NAME - 1);
	end->vars = NULL;

	return end;
}

sqlvarlist *sql_varlist_add_null(sqlvarlist *l)
{
	sqlvar v;
	sql_var_set_default(&v, SQL_NULL);
	return sql_varlist_add_varstruct(l, &v);
}

sqlvarlist *sql_varlist_add_varchar(sqlvarlist *l, const void *data, sqlsize s)
{
	sqlvar v;
	sql_var_set_default(&v, SQL_VARCHAR);
	strncpy(v.sdata, data, SQL_NUM_ELEMENTS(SQL_VAR_TYPE(sdata)) - 1);
	v.size = s >= MAX_DATA ? MAX_DATA : s + 1;
	return sql_varlist_add_varstruct(l, &v);
}

sqlvarlist *sql_varlist_add_float(sqlvarlist *l, double data)
{
	sqlvar v;
	sql_var_set_default(&v, SQL_FLOAT);
	v.fdata = data;
	return sql_varlist_add_varstruct(l, &v);
}

sqlvarlist *sql_varlist_add_real(sqlvarlist *l, float data)
{
	sqlvar v;
	sql_var_set_default(&v, SQL_REAL);
	v.rdata = data;
	return sql_varlist_add_varstruct(l, &v);
}

sqlvarlist *sql_varlist_add_int(sqlvarlist *l, int data)
{
	sqlvar v;
	sql_var_set_default(&v, SQL_INT);
	v.idata = data;
	return sql_varlist_add_varstruct(l, &v);
}

sqlvarlist *sql_varlist_add_bigint(sqlvarlist *l, long long data)
{
	sqlvar v;
	sql_var_set_default(&v, SQL_BIGINT);
	v.lldata = data;
	return sql_varlist_add_varstruct(l, &v);
}

sqlvarlist *sql_varlist_add_varstruct(sqlvarlist *l, const sqlvar *v)
{
	size_t end = 0;
	if (l) {
		end = l->list.nvars;
	}
	else {
		l = sql_list_new_varlist("root");
	}

	sqlvar *r = realloc(l->vars, end * sizeof(sqlvar) + sizeof(sqlvar));
	if (!r) {
		return NULL;
	}

	l->vars = r;
	l->list.nvars = end + 1;
	l->vars[end].type = v->type;
	l->vars[end].size = v->size;
	memcpy(l->vars[end].data, v->data, sizeof(SQL_VAR_TYPE(data)));

	return l;
}

void sql_varlist_free(sqlvarlist *l)
{
	sqlvarlist *cur_list = l;
	while(cur_list) {
		if (cur_list->vars) {
			free(cur_list->vars);
		}
		cur_list = (sqlvarlist *)sql_list_get_next_list(&cur_list->list);
	}
	free(l);
}

void sql_list_nullfree(sqllist * l)
{
	(void)l;
}

sqlptrlist *sql_list_new_ptrlist(sqllist *vlist, sqlvar *vars, size_t nvars, sqllist_free free_function)
{
	sqlptrlist *n = NULL;
	return sql_ptrlist_append_ptrlist(&n, vlist, vars, nvars, free_function);
}

sqlptrlist *sql_ptrlist_append_ptrlist(sqlptrlist **b, sqllist *vlist, sqlvar *vars, size_t nvars, sqllist_free free_function)
{
	size_t total_size = 0;
	sqlptrlist *cur_list = NULL;
	if (*b) {
		sqllist *list_next = &(*b)->list;
		do {
			cur_list = (sqlptrlist *)list_next;
			total_size += sizeof(sqlptrlist);
			list_next = sql_list_get_next_list(list_next);
		} while (list_next);
	}

	if (cur_list) {
		cur_list->list.next_list = sizeof(sqlptrlist);
	}

	sqlptrlist *r = realloc(*b, total_size + sizeof(sqlptrlist));
	if (!r) {
		if (cur_list) {
			cur_list->list.next_list = 0;
		}
		return NULL;
	}
	*b = r;
	sqlptrlist *end = (sqlptrlist*)(((uint8_t*)r) + total_size);

	end->list.nvars = nvars;
	end->list.next_list = 0;
	memset(end->list.id, 0, MAX_NAME);
	strncpy(end->list.id, vlist->id, MAX_NAME - 1);
	end->vars = vars;
	end->vlist = vlist;
	end->free_vlist = free_function;

	return end;
}

void sql_ptrlist_free(sqlptrlist *l)
{
	l->free_vlist(l->vlist);
	free(l);
}

sqllist *sql_list_get_next_list(const sqllist *l)
{
	return (l && l->next_list) ? (sqllist*)(((const uint8_t*)l) + l->next_list) : NULL;
}

sqllist *sql_list_get_list_id(sqllist *l, const char *id)
{
	while (l && strcmp(id, l->id)) {
		l = sql_list_get_next_list(l);
	}
	return l;
}

sqlsize sql_strlen(const char *name, size_t size)
{
	sqlsize s = 0;
	while (*name != '\0' && (size == 0 || s < size)) {
		s++;
		name++;
	}
	return s;
}

static void appmain_print_name(const sqlvar *vname)
{
	char name[MAX_DATA];
	memset(name, 0, sizeof(name));

	switch (vname->type)
	{
	case SQL_VARCHAR:
		sql_var_get_cstring(vname, name, sizeof(name) - 1);
		appmain_send_to_client("%s\t", name);
		break;
	default:
		appmain_send_to_client("Unexpected type: %d with value ", vname->type);
		for (size_t j = 0; j < SQL_NUM_ELEMENTS(SQL_VAR_TYPE(data)); j++) {
			appmain_send_to_client("%02X", vname->data[j]);
		}
		appmain_send_to_client("\n");
		break;
	}
}

static void appmain_print_price(const sqlvar *vprice)
{
	switch (vprice->type)
	{
	case SQL_BIGINT:
		appmain_send_to_client("$%0.2lf\t", sql_var_get_bigint(vprice) / 100.0);
		break;
	default:
		appmain_send_to_client("Unexpected type: %d with value ", vprice->type);
		for (size_t j = 0; j < SQL_NUM_ELEMENTS(SQL_VAR_TYPE(data)); j++) {
			appmain_send_to_client("%02X", vprice->data[j]);
		}
		appmain_send_to_client("\n");
		break;
	}
}

void appmain_print_ingredients(const sqlvarlist *l)
{
	if (!l) {
		return;
	}

	size_t max_size = 0;
	const sqlvarlist *c = l;
	while (c) {
		if (c->list.nvars > max_size) {
			max_size = c->list.nvars;
		}
		appmain_send_to_client("%s\t", c->list.id);
		c = (const sqlvarlist *)sql_list_get_next_list(&c->list);
	}

	for (size_t i = 0; i < max_size; i++) {
		appmain_send_to_client("\n");
		c = l;
		while (c) {
			if (i < c->list.nvars) {
				if (!strncmp(c->list.id, "price", sizeof(c->list.id))) {
					appmain_print_price(&c->vars[i]);
				}
				else if (!strncmp(c->list.id, "name", sizeof(c->list.id))) {
					appmain_print_name(&c->vars[i]);
				}
				else {
					appmain_send_to_client("Unexpected column!\n");
				}
			}
			c = (const sqlvarlist *)sql_list_get_next_list(&c->list);
		}
	}
}

sqlconditional *sql_conditional_append_conditional(sqlconditional **c, sqllogic logic_op)
{
	size_t total_size = 0;
	sqlconditional *cur_cond = NULL;
	if (*c) {
		sqlconditional *next_cond = *c;
		do {
			cur_cond = next_cond;
			total_size += sizeof(sqlconditional);
			next_cond = sql_conditional_get_next_conditional(next_cond);
		} while (next_cond);
	}

	if (cur_cond) {
		cur_cond->next_condition = sizeof(sqlconditional);
	}

	sqlconditional *r = realloc(*c, total_size + sizeof(sqlconditional));
	if (!r) {
		if (cur_cond) {
			cur_cond->next_condition = 0;
		}
		return NULL;
	}
	*c = r;

	sqlconditional *end = (sqlconditional*)(((uint8_t*)r) + total_size);

	end->next_condition = 0;
	end->bool_operation = SQL_BOOLEAN_NONE;
	end->logic_operation = logic_op;

	return end;
}

void sql_conditional_free(sqlconditional *c)
{
	free(c);
}

sqlconditional *sql_conditional_get_next_conditional(const sqlconditional *c)
{
	return (c && c->next_condition) ? (sqlconditional*)(((const uint8_t*)c) + c->next_condition) : NULL;
}

sqlassignment *sql_assignment_append_assignment(sqlassignment **c, sqlvar *lhs, sqlformula *rhs)
{
	size_t total_size = 0;
	sqlassignment *cur_assign = NULL;
	if (*c) {
		sqlassignment *next_assign = *c;
		do {
			cur_assign = next_assign;
			total_size += sizeof(sqlassignment);
			next_assign = sql_assignment_get_next_assignment(next_assign);
		} while (next_assign);
	}

	if (cur_assign) {
		cur_assign->next_assignment = sizeof(sqlassignment);
	}

	sqlassignment *r = realloc(*c, total_size + sizeof(sqlassignment));
	if (!r) {
		if (cur_assign) {
			cur_assign->next_assignment = 0;
		}
		return NULL;
	}
	*c = r;

	sqlassignment *end = (sqlassignment*)(((uint8_t*)r) + total_size);

	end->next_assignment = 0;
	end->right = rhs;
	sql_var_set_varstruct(&end->left, lhs);

	return end;
}

void sql_assignment_free(sqlassignment *c)
{
	if (c->right) {
		sql_formula_free(c->right);
		c->right = NULL;
	}

	free(c);
}

sqlassignment *sql_assignment_get_next_assignment(const sqlassignment *c)
{
	return (c && c->next_assignment) ? (sqlassignment*)(((const uint8_t*)c) + c->next_assignment) : NULL;
}

void sql_formula_insert_formula(sqlformula **f, sqlarithmetic arithmetic_op)
{
	sqlformula *head = malloc(sizeof(sqlformula));
	head->right = *f;
	head->arithmetic_operation = arithmetic_op;
	*f = head;
}

void sql_formula_free(sqlformula *f)
{
	while (f) {
		sqlformula *next = f->right;
		free(f);
		f = next;
	}
}

sqlformula *sql_formula_get_next_formula(const sqlformula *f)
{
	return f ? f->right : NULL;
}
