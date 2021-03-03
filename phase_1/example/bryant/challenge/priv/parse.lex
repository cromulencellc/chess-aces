%{
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lex.h"

#include "parse.tab.h"

char* unquote_identifier(char* text);
char* unquote_character_literal(char* text);
 char* unquote_parameter(char* text);

%}

%option noyywrap
%option nounput
%option noinput

%%
(?i:select) { return SELECT; }
(?i:from) { return FROM; }
(?i:where) { return WHERE; }
(?i:union) { return UNION; }

\( { return LPAREN; }
\) { return RPAREN; }

\+ { return PLUS; }
\- { return MINUS; }
\* { return ASTERISK; }
\/ { return SOLIDUS; }

\= { return EQ; }
\<\> { return NEQ; }
\< { return LT; }
\<= { return LTEQ; }
\> { return GT; }
\>= { return GTEQ; }

(?i:not) { return NOT; }
(?i:or) { return OR; }
(?i:and) { return AND; }

\, { return COMMA; }
\; { return SEMICOLON; }

'([^']|'')*' { yylval.str = unquote_character_literal(yytext); return CHARACTER_LITERAL; }
[a-zA-Z][a-zA-Z0-9]* { yylval.str = strdup(yytext); return IDENTIFIER; }
\"(\"\"|[^\"])*\" { yylval.str = unquote_identifier(yytext); return IDENTIFIER; }

:[a-zA-Z][a-zA-Z0-9]* { yylval.str = unquote_parameter(yytext); return PARAMETER; }

[ \t\v\n\f] /* noop */
--[^\n]* /* comment, noop */
%%

char* unquote_identifier(char* text) {
  char* fixed = calloc(strlen(text), sizeof(char));

  size_t cur = 1; // skip open quote
  size_t fixed_cur = 0;

  while (text[cur] != 0) {
    if (('\"' == text[cur]) && ('\"' == text[cur + 1])) {
      fixed[fixed_cur] = '"';
      cur += 2;
      fixed_cur += 1;
    } else if ('\"' == text[cur]) {
      return fixed;
    } else {
      fixed[fixed_cur] = text[cur];
      cur++;
      fixed_cur++;
    }
  }

  assert(false);
}

char* unquote_character_literal(char* text) {
  char* fixed = calloc(strlen(text), sizeof(char));

  size_t cur = 1; // skip open quote
  size_t fixed_cur = 0;

  while (text[cur] != 0) {
    if (('\'' == text[cur]) && ('\'' == text[cur + 1])) {
      fixed[fixed_cur] = '"';
      cur += 2;
      fixed_cur += 1;
    } else if ('\'' == text[cur]) {
      return fixed;
    } else {
      fixed[fixed_cur] = text[cur];
      cur++;
      fixed_cur++;
    }
  }

  assert(false);
}

char* unquote_parameter(char* text) {
  char* fixed = strdup(text + 1);
  return fixed;
}

ast* parse(char* query) {
    parsed_query = NULL;

    yy_scan_string(query);
    yyparse();

  return parsed_query;
}
