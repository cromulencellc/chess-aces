
#ifndef YY_YY_TEMP_TAB_H_INCLUDED
#define YY_YY_TEMP_TAB_H_INCLUDED
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
#ifndef YYTOKENTYPE
#define YYTOKENTYPE
enum yytokentype {
  NAME = 258,
  PLUS = 259,
  MINUS = 260,
  MULTIPLY = 261,
  LPAREN = 262,
  RPAREN = 263,
  ASSIGN = 264,
  SINGLE_QUOTE = 265,
  NEWLINE = 266,
  QUIT = 267,
  COMMA = 268,
  HAT = 269,
  DOT_HAT = 270,
  DOT_DIVIDE = 271,
  DOT_MULTIPLY = 272,
  BARS = 273,
  MATRIX = 274,
  FUNCTION = 275,
  DIVIDE = 276
};
#endif
#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED
union YYSTYPE {
#line 33 "temp.y"
  char *var;
  char *func;
  struct matrix *m;
#line 82 "temp.tab.h"
};
typedef union YYSTYPE YYSTYPE;
#define YYSTYPE_IS_TRIVIAL 1
#define YYSTYPE_IS_DECLARED 1
#endif
extern YYSTYPE yylval;
int yyparse(void);
#endif
