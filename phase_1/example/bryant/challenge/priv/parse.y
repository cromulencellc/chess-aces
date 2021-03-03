%{
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "lex.h"
#include "log.h"

#define YYPARSE_PARAM scanner
#define YYLEX_PARAM   scanner
%}


%union {
    int intval;
    char* str;
    ast* a;
    query_term* qt;
}

%type   <a>             query select_list select_sublist
%type   <qt>            query_term table_expression
%type   <str>           from_clause
%type   <a>             where_clause
%type   <a>             boolean_expression boolean_term boolean_factor boolean_test comparison_predicate
%type   <a>             value

%type   <intval>           comparison

%token SELECT
%token FROM
%token WHERE
%token UNION

%token LPAREN
%token RPAREN

%token PLUS
%token MINUS
%token ASTERISK
%token SOLIDUS

%token EQ
%token NEQ
%token LT
%token LTEQ
%token GT
%token GTEQ

%token NOT
%token OR
%token AND

%token  <str> CHARACTER_LITERAL
%token  <str> IDENTIFIER
%token  <str>           PARAMETER

%token COMMA
%token SEMICOLON

%%
input:
                query SEMICOLON { parsed_query = $1; }
        |       query { parsed_query = $1; }
        ;

query:
          query_term { $$ = ast_create('t', (ast*)(void*)$1, NULL); }
|       query_term UNION query { $$ = ast_create('t', (ast*)(void*)$1, (ast*)(void*)$3); }
        ;

query_term:
               SELECT select_list table_expression { $$ = qt_create($2, $3); }
        ;

select_list:
                ASTERISK { $$ = ast_create('*', NULL, NULL); }
        |       select_sublist { $$ = $1; }
        ;

select_sublist:
                IDENTIFIER {
                    $$ = ast_create('s', (ast*)(void*) $1, NULL);
                }
        |       IDENTIFIER COMMA select_sublist {
            $$ = ast_create('s', (ast*)(void*) $1, $3);
                }
        ;

table_expression:
                     from_clause where_clause { $$ = te_create($1, $2); }
        ;

from_clause:
                FROM IDENTIFIER { $$ = $2; }
        ;

where_clause:
                %empty { $$ = NULL; }
        |       WHERE boolean_expression { $$ = $2; }
        ;

boolean_expression:
                boolean_term { $$ = $1; }
        |       boolean_expression OR boolean_term {
            $$ = ast_create('|', $1, $3);
                }
        ;

boolean_term:
                boolean_factor { $$ = $1; }
        |       boolean_term AND boolean_factor {
            $$ = ast_create('&', $1, $3);
                }
        ;

boolean_factor:
                boolean_test { $$ = $1; }
        |       NOT boolean_test { $$ = ast_create('!', $2, NULL); }
        ;

boolean_test:
                comparison_predicate { $$ = $1; }
        |       LPAREN boolean_expression RPAREN { $$ = $2; }
        ;

comparison_predicate:
                         value comparison value { $$ = ast_create($2, $1, $3); }
        ;

value:
          IDENTIFIER { $$ = ast_create('i', (ast*)(void*)$1, NULL);}
        |       CHARACTER_LITERAL { $$ = ast_create('l', (ast*)(void*)$1, NULL); }
|       PARAMETER { $$ = ast_create('p', (ast*)(void*)$1, NULL); }
        ;

comparison:
               EQ { $$ = '='; }
        |       NEQ { $$ = '\\'; }
        |       LT { $$ = '<'; }
        |       LTEQ { $$ = ','; }
        |       GT { $$ = '>'; }
        |       GTEQ { $$ = '.'; }
        ;


%%


ast* ast_create(int nt, ast* l, ast* r) {
    ast* a = calloc(1, sizeof(ast));
    lll("ast(%c, %p, %p) = %p\n", (char)nt, l, r, a);
    a->nodetype = nt;
    a->l = l;
    a->r = r;

    return a;
}

query_term* qt_create(ast* select_list, query_term* table_expression) {
    table_expression->select = select_list;

    lll("query_term(select %p from %s where %p) = %p\n",
        select_list,
        table_expression->from,
        table_expression->where,
        table_expression);
    return table_expression;
}

query_term* te_create(char* from, ast* where_clause) {
    query_term* te = calloc(1, sizeof(query_term));
    te->from = from;
    te->where = where_clause;
    lll("query_term(from %s where %p) = %p\n", from, where_clause, te);
    return te;
}

void yyerror (char const *msg) {
    lll("error %s\n", msg);
    exit(-1);
}
