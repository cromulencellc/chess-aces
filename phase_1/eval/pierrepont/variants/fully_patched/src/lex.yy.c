
#line 3 "lex.yy.c"
#define YY_INT_ALIGNED short int
#define FLEX_SCANNER
#define YY_FLEX_MAJOR_VERSION 2
#define YY_FLEX_MINOR_VERSION 6
#define YY_FLEX_SUBMINOR_VERSION 4
#if YY_FLEX_SUBMINOR_VERSION > 0
#define FLEX_BETA
#endif
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef FLEXINT_H
#define FLEXINT_H
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS 1
#endif
#include <inttypes.h>
typedef int8_t flex_int8_t;
typedef uint8_t flex_uint8_t;
typedef int16_t flex_int16_t;
typedef uint16_t flex_uint16_t;
typedef int32_t flex_int32_t;
typedef uint32_t flex_uint32_t;
#else
typedef signed char flex_int8_t;
typedef short int flex_int16_t;
typedef int flex_int32_t;
typedef unsigned char flex_uint8_t;
typedef unsigned short int flex_uint16_t;
typedef unsigned int flex_uint32_t;
#ifndef INT8_MIN
#define INT8_MIN (-128)
#endif
#ifndef INT16_MIN
#define INT16_MIN (-32767 - 1)
#endif
#ifndef INT32_MIN
#define INT32_MIN (-2147483647 - 1)
#endif
#ifndef INT8_MAX
#define INT8_MAX (127)
#endif
#ifndef INT16_MAX
#define INT16_MAX (32767)
#endif
#ifndef INT32_MAX
#define INT32_MAX (2147483647)
#endif
#ifndef UINT8_MAX
#define UINT8_MAX (255U)
#endif
#ifndef UINT16_MAX
#define UINT16_MAX (65535U)
#endif
#ifndef UINT32_MAX
#define UINT32_MAX (4294967295U)
#endif
#ifndef SIZE_MAX
#define SIZE_MAX (~(size_t)0)
#endif
#endif
#endif
#define yyconst const
#if defined(__GNUC__) && __GNUC__ >= 3
#define yynoreturn __attribute__((__noreturn__))
#else
#define yynoreturn
#endif
#define YY_NULL 0
#define YY_SC_TO_UI(c) ((YY_CHAR)(c))
#define BEGIN (yy_start) = 1 + 2 *
#define YY_START (((yy_start)-1) / 2)
#define YYSTATE YY_START
#define YY_STATE_EOF(state) (YY_END_OF_BUFFER + state + 1)
#define YY_NEW_FILE yyrestart(yyin)
#define YY_END_OF_BUFFER_CHAR 0
#ifndef YY_BUF_SIZE
#ifdef __ia64__
#define YY_BUF_SIZE 32768
#else
#define YY_BUF_SIZE 16384
#endif
#endif
#define YY_STATE_BUF_SIZE ((YY_BUF_SIZE + 2) * sizeof(yy_state_type))
#ifndef YY_TYPEDEF_YY_BUFFER_STATE
#define YY_TYPEDEF_YY_BUFFER_STATE
typedef struct yy_buffer_state *YY_BUFFER_STATE;
#endif
#ifndef YY_TYPEDEF_YY_SIZE_T
#define YY_TYPEDEF_YY_SIZE_T
typedef size_t yy_size_t;
#endif
extern int yyleng;
extern FILE *yyin, *yyout;
#define EOB_ACT_CONTINUE_SCAN 0
#define EOB_ACT_END_OF_FILE 1
#define EOB_ACT_LAST_MATCH 2
#define YY_LESS_LINENO(n)
#define YY_LINENO_REWIND_TO(ptr)
#define yyless(n)                                                              \
  do {                                                                         \
    int yyless_macro_arg = (n);                                                \
    YY_LESS_LINENO(yyless_macro_arg);                                          \
    *yy_cp = (yy_hold_char);                                                   \
    YY_RESTORE_YY_MORE_OFFSET(yy_c_buf_p) = yy_cp =                            \
        yy_bp + yyless_macro_arg - YY_MORE_ADJ;                                \
    YY_DO_BEFORE_ACTION;                                                       \
  } while (0)
#define unput(c) yyunput(c, (yytext_ptr))
#ifndef YY_STRUCT_YY_BUFFER_STATE
#define YY_STRUCT_YY_BUFFER_STATE
struct yy_buffer_state {
  FILE *yy_input_file;
  char *yy_ch_buf;
  char *yy_buf_pos;
  int yy_buf_size;
  int yy_n_chars;
  int yy_is_our_buffer;
  int yy_is_interactive;
  int yy_at_bol;
  int yy_bs_lineno;
  int yy_bs_column;
  int yy_fill_buffer;
  int yy_buffer_status;
#define YY_BUFFER_NEW 0
#define YY_BUFFER_NORMAL 1
#define YY_BUFFER_EOF_PENDING 2
};
#endif
static size_t yy_buffer_stack_top = 0;
static size_t yy_buffer_stack_max = 0;
static YY_BUFFER_STATE *yy_buffer_stack = NULL;
#define YY_CURRENT_BUFFER                                                      \
  ((yy_buffer_stack) ? (yy_buffer_stack)[(yy_buffer_stack_top)] : NULL)
#define YY_CURRENT_BUFFER_LVALUE (yy_buffer_stack)[(yy_buffer_stack_top)]
static char yy_hold_char;
static int yy_n_chars;
int yyleng;
static char *yy_c_buf_p = NULL;
static int yy_init = 0;
static int yy_start = 0;
static int yy_did_buffer_switch_on_eof;
void yyrestart(FILE *input_file);
void yy_switch_to_buffer(YY_BUFFER_STATE new_buffer);
YY_BUFFER_STATE yy_create_buffer(FILE *file, int size);
void yy_delete_buffer(YY_BUFFER_STATE b);
void yy_flush_buffer(YY_BUFFER_STATE b);
void yypush_buffer_state(YY_BUFFER_STATE new_buffer);
void yypop_buffer_state(void);
static void yyensure_buffer_stack(void);
static void yy_load_buffer_state(void);
static void yy_init_buffer(YY_BUFFER_STATE b, FILE *file);
#define YY_FLUSH_BUFFER yy_flush_buffer(YY_CURRENT_BUFFER)
YY_BUFFER_STATE yy_scan_buffer(char *base, yy_size_t size);
YY_BUFFER_STATE yy_scan_string(const char *yy_str);
YY_BUFFER_STATE yy_scan_bytes(const char *bytes, int len);
void *yyalloc(yy_size_t);
void *yyrealloc(void *, yy_size_t);
void yyfree(void *);
#define yy_new_buffer yy_create_buffer
#define yy_set_interactive(is_interactive)                                     \
  {                                                                            \
    if (!YY_CURRENT_BUFFER) {                                                  \
      yyensure_buffer_stack();                                                 \
      YY_CURRENT_BUFFER_LVALUE = yy_create_buffer(yyin, YY_BUF_SIZE);          \
    }                                                                          \
    YY_CURRENT_BUFFER_LVALUE->yy_is_interactive = is_interactive;              \
  }
#define yy_set_bol(at_bol)                                                     \
  {                                                                            \
    if (!YY_CURRENT_BUFFER) {                                                  \
      yyensure_buffer_stack();                                                 \
      YY_CURRENT_BUFFER_LVALUE = yy_create_buffer(yyin, YY_BUF_SIZE);          \
    }                                                                          \
    YY_CURRENT_BUFFER_LVALUE->yy_at_bol = at_bol;                              \
  }
#define YY_AT_BOL() (YY_CURRENT_BUFFER_LVALUE->yy_at_bol)
#define yywrap() (1)
#define YY_SKIP_YYWRAP
typedef flex_uint8_t YY_CHAR;
FILE *yyin = NULL, *yyout = NULL;
typedef int yy_state_type;
extern int yylineno;
int yylineno = 1;
extern char *yytext;
#ifdef yytext_ptr
#undef yytext_ptr
#endif
#define yytext_ptr yytext
static yy_state_type yy_get_previous_state(void);
static yy_state_type yy_try_NUL_trans(yy_state_type current_state);
static int yy_get_next_buffer(void);
static void yynoreturn yy_fatal_error(const char *msg);
#define YY_DO_BEFORE_ACTION                                                    \
  (yytext_ptr) = yy_bp;                                                        \
  yyleng = (int)(yy_cp - yy_bp);                                               \
  (yy_hold_char) = *yy_cp;                                                     \
  *yy_cp = '\0';                                                               \
  (yy_c_buf_p) = yy_cp;
#define YY_NUM_RULES 22
#define YY_END_OF_BUFFER 23
struct yy_trans_info {
  flex_int32_t yy_verify;
  flex_int32_t yy_nxt;
};
static const flex_int16_t yy_accept[45] = {
    0,  0, 0, 23, 22, 4,  8,  22, 17, 15, 16, 13, 10, 18, 11,
    22, 6, 9, 3,  22, 20, 3,  22, 2,  0,  6,  12, 14, 19, 0,
    3,  3, 0, 0,  0,  3,  21, 5,  0,  0,  7,  3,  0,  1,  0};
static const YY_CHAR yy_ec[256] = {
    0,  1,  1,  1,  1,  1,  1,  1,  1,  2,  3,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  4,  1,  1,  1,  1,  5,
    1,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 1,  16, 1,  17, 1,  1,  1,  18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,
    18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 1,  20, 21,
    1,  1,  18, 18, 18, 18, 22, 18, 18, 18, 23, 18, 18, 18, 18, 18, 18, 18, 18,
    18, 18, 24, 18, 18, 18, 25, 18, 18, 1,  26, 1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1};
static const YY_CHAR yy_meta[27] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                    1, 2, 1, 1, 3, 1, 1, 1, 3, 3, 3, 3, 1};
static const flex_int16_t yy_base[47] = {
    0,  0,  0,  69, 70, 70, 70, 0,  70, 70, 70, 70, 70, 70, 15, 19,
    16, 70, 53, 37, 70, 17, 41, 0,  22, 23, 70, 70, 70, 51, 50, 49,
    0,  31, 31, 30, 70, 48, 58, 57, 70, 24, 42, 45, 70, 53, 56};
static const flex_int16_t yy_def[47] = {
    0,  44, 1,  44, 44, 44, 44, 45, 44, 44, 44, 44, 44, 44, 44, 44,
    44, 44, 46, 44, 44, 46, 44, 45, 44, 44, 44, 44, 44, 44, 44, 46,
    19, 44, 19, 46, 44, 44, 34, 19, 44, 46, 44, 46, 0,  44, 44};
static const flex_int16_t yy_nxt[97] = {
    0,  4,  5,  6,  5,  7,  8,  9,  10, 11, 12, 13, 14, 15, 4,  16, 4,
    17, 18, 19, 4,  20, 21, 18, 18, 18, 22, 24, 26, 29, 25, 25, 30, 27,
    24, 38, 29, 25, 25, 30, 28, 32, 35, 33, 34, 30, 34, 39, 43, 33, 34,
    40, 34, 41, 33, 34, 23, 34, 31, 31, 30, 42, 44, 37, 30, 30, 37, 36,
    30, 44, 3,  44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
    44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44};
static const flex_int16_t yy_chk[97] = {
    0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  14, 15, 16, 14, 16, 21, 15,
    24, 34, 25, 24, 25, 41, 15, 19, 21, 33, 33, 35, 33, 34, 41, 19, 19,
    34, 19, 35, 42, 42, 45, 42, 46, 46, 43, 39, 38, 37, 31, 30, 29, 22,
    18, 3,  44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
    44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44};
static yy_state_type yy_last_accepting_state;
static char *yy_last_accepting_cpos;
extern int yy_flex_debug;
int yy_flex_debug = 0;
#define REJECT reject_used_but_not_detected
#define yymore() yymore_used_but_not_detected
#define YY_MORE_ADJ 0
#define YY_RESTORE_YY_MORE_OFFSET
char *yytext;
#line 1 "temp.l"
#line 5 "temp.l"
#include <ctype.h>
#include <stdio.h>
#define YY_DECL int yylex()
#include "matrix.h"
#include "temp.tab.h"
#line 488 "lex.yy.c"
#line 489 "lex.yy.c"
#define INITIAL 0
#ifndef YY_NO_UNISTD_H
#include <unistd.h>
#endif
#ifndef YY_EXTRA_TYPE
#define YY_EXTRA_TYPE void *
#endif
static int yy_init_globals(void);
int yylex_destroy(void);
int yyget_debug(void);
void yyset_debug(int debug_flag);
YY_EXTRA_TYPE yyget_extra(void);
void yyset_extra(YY_EXTRA_TYPE user_defined);
FILE *yyget_in(void);
void yyset_in(FILE *_in_str);
FILE *yyget_out(void);
void yyset_out(FILE *_out_str);
int yyget_leng(void);
char *yyget_text(void);
int yyget_lineno(void);
void yyset_lineno(int _line_number);
#ifndef YY_SKIP_YYWRAP
#ifdef __cplusplus
extern "C" int yywrap(void);
#else
extern int yywrap(void);
#endif
#endif
#ifndef yytext_ptr
static void yy_flex_strncpy(char *, const char *, int);
#endif
#ifdef YY_NEED_STRLEN
static int yy_flex_strlen(const char *);
#endif
#ifndef YY_READ_BUF_SIZE
#ifdef __ia64__
#define YY_READ_BUF_SIZE 16384
#else
#define YY_READ_BUF_SIZE 8192
#endif
#endif
#ifndef ECHO
#define ECHO                                                                   \
  do {                                                                         \
    if (fwrite(yytext, (size_t)yyleng, 1, yyout)) {                            \
    }                                                                          \
  } while (0)
#endif
#ifndef YY_INPUT
#define YY_INPUT(buf, result, max_size)                                        \
  if (YY_CURRENT_BUFFER_LVALUE->yy_is_interactive) {                           \
    int c = '*';                                                               \
    int n;                                                                     \
    for (n = 0; n < max_size && (c = getc(yyin)) != EOF && c != '\n'; ++n)     \
      buf[n] = (char)c;                                                        \
    if (c == '\n')                                                             \
      buf[n++] = (char)c;                                                      \
    if (c == EOF && ferror(yyin))                                              \
      YY_FATAL_ERROR("input in flex scanner failed");                          \
    result = n;                                                                \
  } else {                                                                     \
    errno = 0;                                                                 \
    while ((result = (int)fread(buf, 1, (yy_size_t)max_size, yyin)) == 0 &&    \
           ferror(yyin)) {                                                     \
      if (errno != EINTR) {                                                    \
        YY_FATAL_ERROR("input in flex scanner failed");                        \
        break;                                                                 \
      }                                                                        \
      errno = 0;                                                               \
      clearerr(yyin);                                                          \
    }                                                                          \
  }
#endif
#ifndef yyterminate
#define yyterminate() return YY_NULL
#endif
#ifndef YY_START_STACK_INCR
#define YY_START_STACK_INCR 25
#endif
#ifndef YY_FATAL_ERROR
#define YY_FATAL_ERROR(msg) yy_fatal_error(msg)
#endif
#ifndef YY_DECL
#define YY_DECL_IS_OURS 1
extern int yylex(void);
#define YY_DECL int yylex(void)
#endif
#ifndef YY_USER_ACTION
#define YY_USER_ACTION
#endif
#ifndef YY_BREAK
#define YY_BREAK break;
#endif
#define YY_RULE_SETUP YY_USER_ACTION
YY_DECL {
  yy_state_type yy_current_state;
  char *yy_cp, *yy_bp;
  int yy_act;
  if (!(yy_init)) {
    (yy_init) = 1;
#ifdef YY_USER_INIT
    YY_USER_INIT;
#endif
    if (!(yy_start))
      (yy_start) = 1;
    if (!yyin)
      yyin = stdin;
    if (!yyout)
      yyout = stdout;
    if (!YY_CURRENT_BUFFER) {
      yyensure_buffer_stack();
      YY_CURRENT_BUFFER_LVALUE = yy_create_buffer(yyin, YY_BUF_SIZE);
    }
    yy_load_buffer_state();
  }
  {
#line 15 "temp.l"
#line 710 "lex.yy.c"
    while (1) {
      yy_cp = (yy_c_buf_p);
      *yy_cp = (yy_hold_char);
      yy_bp = yy_cp;
      yy_current_state = (yy_start);
    yy_match:
      do {
        YY_CHAR yy_c = yy_ec[YY_SC_TO_UI(*yy_cp)];
        if (yy_accept[yy_current_state]) {
          (yy_last_accepting_state) = yy_current_state;
          (yy_last_accepting_cpos) = yy_cp;
        }
        while (yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state) {
          yy_current_state = (int)yy_def[yy_current_state];
          if (yy_current_state >= 45)
            yy_c = yy_meta[yy_c];
        }
        yy_current_state = yy_nxt[yy_base[yy_current_state] + yy_c];
        ++yy_cp;
      } while (yy_base[yy_current_state] != 70);
    yy_find_action:
      yy_act = yy_accept[yy_current_state];
      if (yy_act == 0) {
        yy_cp = (yy_last_accepting_cpos);
        yy_current_state = (yy_last_accepting_state);
        yy_act = yy_accept[yy_current_state];
      }
      YY_DO_BEFORE_ACTION;
    do_action:
      switch (yy_act) {
      case 0:
        *yy_cp = (yy_hold_char);
        yy_cp = (yy_last_accepting_cpos);
        yy_current_state = (yy_last_accepting_state);
        goto yy_find_action;
      case 1:
        YY_RULE_SETUP
#line 18 "temp.l"
        {
          return QUIT;
        }
        YY_BREAK
      case 2:
        YY_RULE_SETUP
#line 20 "temp.l"
        {
          yylval.func = strdup(yytext);
          return FUNCTION;
        }
        YY_BREAK
      case 3:
        YY_RULE_SETUP
#line 26 "temp.l"
        {
          char *sp = strdup(yytext);
          yylval.var = sp;
          return NAME;
        }
        YY_BREAK
      case 4:
        YY_RULE_SETUP
#line 31 "temp.l"
            ;
        YY_BREAK
      case 5:
        YY_RULE_SETUP
#line 32 "temp.l"
        {
          yylval.m = scalar_to_matrix(strtold(yytext, NULL));
          return MATRIX;
        }
        YY_BREAK
      case 6:
        YY_RULE_SETUP
#line 36 "temp.l"
        {
          yylval.m = scalar_to_matrix(strtold(yytext, NULL));
          if (yylval.m) {
          }
          return MATRIX;
        }
        YY_BREAK
      case 7:
        YY_RULE_SETUP
#line 43 "temp.l"
        {
          char *m = strdup(yytext);
          yylval.m = parse_matrix(m);
          free(m);
          return MATRIX;
        }
        YY_BREAK
      case 8:
        YY_RULE_SETUP
#line 53 "temp.l"
        {
          return NEWLINE;
        }
        YY_BREAK
      case 9:
        YY_RULE_SETUP
#line 54 "temp.l"
        {
          return ASSIGN;
        }
        YY_BREAK
      case 10:
        YY_RULE_SETUP
#line 55 "temp.l"
        {
          return PLUS;
        }
        YY_BREAK
      case 11:
        YY_RULE_SETUP
#line 56 "temp.l"
        {
          return MINUS;
        }
        YY_BREAK
      case 12:
        YY_RULE_SETUP
#line 57 "temp.l"
        {
          return DOT_MULTIPLY;
        }
        YY_BREAK
      case 13:
        YY_RULE_SETUP
#line 58 "temp.l"
        {
          return MULTIPLY;
        }
        YY_BREAK
      case 14:
        YY_RULE_SETUP
#line 59 "temp.l"
        {
          return DOT_DIVIDE;
        }
        YY_BREAK
      case 15:
        YY_RULE_SETUP
#line 60 "temp.l"
        {
          return LPAREN;
        }
        YY_BREAK
      case 16:
        YY_RULE_SETUP
#line 61 "temp.l"
        {
          return RPAREN;
        }
        YY_BREAK
      case 17:
        YY_RULE_SETUP
#line 62 "temp.l"
        {
          return SINGLE_QUOTE;
        }
        YY_BREAK
      case 18:
        YY_RULE_SETUP
#line 63 "temp.l"
        {
          return COMMA;
        }
        YY_BREAK
      case 19:
        YY_RULE_SETUP
#line 64 "temp.l"
        {
          return DOT_HAT;
        }
        YY_BREAK
      case 20:
        YY_RULE_SETUP
#line 65 "temp.l"
        {
          return HAT;
        }
        YY_BREAK
      case 21:
        YY_RULE_SETUP
#line 66 "temp.l"
        {
          return BARS;
        }
        YY_BREAK
      case 22:
        YY_RULE_SETUP
#line 68 "temp.l"
        ECHO;
        YY_BREAK
#line 903 "lex.yy.c"
      case YY_STATE_EOF(INITIAL):
        yyterminate();
      case YY_END_OF_BUFFER: {
        int yy_amount_of_matched_text = (int)(yy_cp - (yytext_ptr)) - 1;
        *yy_cp = (yy_hold_char);
        YY_RESTORE_YY_MORE_OFFSET
        if (YY_CURRENT_BUFFER_LVALUE->yy_buffer_status == YY_BUFFER_NEW) {
          (yy_n_chars) = YY_CURRENT_BUFFER_LVALUE->yy_n_chars;
          YY_CURRENT_BUFFER_LVALUE->yy_input_file = yyin;
          YY_CURRENT_BUFFER_LVALUE->yy_buffer_status = YY_BUFFER_NORMAL;
        }
        if ((yy_c_buf_p) <=
            &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[(yy_n_chars)]) {
          yy_state_type yy_next_state;
          (yy_c_buf_p) = (yytext_ptr) + yy_amount_of_matched_text;
          yy_current_state = yy_get_previous_state();
          yy_next_state = yy_try_NUL_trans(yy_current_state);
          yy_bp = (yytext_ptr) + YY_MORE_ADJ;
          if (yy_next_state) {
            yy_cp = ++(yy_c_buf_p);
            yy_current_state = yy_next_state;
            goto yy_match;
          } else {
            yy_cp = (yy_c_buf_p);
            goto yy_find_action;
          }
        } else
          switch (yy_get_next_buffer()) {
          case EOB_ACT_END_OF_FILE: {
            (yy_did_buffer_switch_on_eof) = 0;
            if (yywrap()) {
              (yy_c_buf_p) = (yytext_ptr) + YY_MORE_ADJ;
              yy_act = YY_STATE_EOF(YY_START);
              goto do_action;
            } else {
              if (!(yy_did_buffer_switch_on_eof))
                YY_NEW_FILE;
            }
            break;
          }
          case EOB_ACT_CONTINUE_SCAN:
            (yy_c_buf_p) = (yytext_ptr) + yy_amount_of_matched_text;
            yy_current_state = yy_get_previous_state();
            yy_cp = (yy_c_buf_p);
            yy_bp = (yytext_ptr) + YY_MORE_ADJ;
            goto yy_match;
          case EOB_ACT_LAST_MATCH:
            (yy_c_buf_p) = &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[(yy_n_chars)];
            yy_current_state = yy_get_previous_state();
            yy_cp = (yy_c_buf_p);
            yy_bp = (yytext_ptr) + YY_MORE_ADJ;
            goto yy_find_action;
          }
        break;
      }
      default:
        YY_FATAL_ERROR("fatal flex scanner internal error--no action found");
      }
    }
  }
}
static int yy_get_next_buffer(void) {
  char *dest = YY_CURRENT_BUFFER_LVALUE->yy_ch_buf;
  char *source = (yytext_ptr);
  int number_to_move, i;
  int ret_val;
  if ((yy_c_buf_p) > &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[(yy_n_chars) + 1])
    YY_FATAL_ERROR("fatal flex scanner internal error--end of buffer missed");
  if (YY_CURRENT_BUFFER_LVALUE->yy_fill_buffer == 0) {
    if ((yy_c_buf_p) - (yytext_ptr)-YY_MORE_ADJ == 1) {
      return EOB_ACT_END_OF_FILE;
    } else {
      return EOB_ACT_LAST_MATCH;
    }
  }
  number_to_move = (int)((yy_c_buf_p) - (yytext_ptr)-1);
  for (i = 0; i < number_to_move; ++i)
    *(dest++) = *(source++);
  if (YY_CURRENT_BUFFER_LVALUE->yy_buffer_status == YY_BUFFER_EOF_PENDING)
    YY_CURRENT_BUFFER_LVALUE->yy_n_chars = (yy_n_chars) = 0;
  else {
    int num_to_read =
        YY_CURRENT_BUFFER_LVALUE->yy_buf_size - number_to_move - 1;
    while (num_to_read <= 0) {
      YY_BUFFER_STATE b = YY_CURRENT_BUFFER_LVALUE;
      int yy_c_buf_p_offset = (int)((yy_c_buf_p)-b->yy_ch_buf);
      if (b->yy_is_our_buffer) {
        int new_size = b->yy_buf_size * 2;
        if (new_size <= 0)
          b->yy_buf_size += b->yy_buf_size / 8;
        else
          b->yy_buf_size *= 2;
        b->yy_ch_buf = (char *)yyrealloc((void *)b->yy_ch_buf,
                                         (yy_size_t)(b->yy_buf_size + 2));
      } else
        b->yy_ch_buf = NULL;
      if (!b->yy_ch_buf)
        YY_FATAL_ERROR("fatal error - scanner input buffer overflow");
      (yy_c_buf_p) = &b->yy_ch_buf[yy_c_buf_p_offset];
      num_to_read = YY_CURRENT_BUFFER_LVALUE->yy_buf_size - number_to_move - 1;
    }
    if (num_to_read > YY_READ_BUF_SIZE)
      num_to_read = YY_READ_BUF_SIZE;
    YY_INPUT((&YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[number_to_move]),
             (yy_n_chars), num_to_read);
    YY_CURRENT_BUFFER_LVALUE->yy_n_chars = (yy_n_chars);
  }
  if ((yy_n_chars) == 0) {
    if (number_to_move == YY_MORE_ADJ) {
      ret_val = EOB_ACT_END_OF_FILE;
      yyrestart(yyin);
    } else {
      ret_val = EOB_ACT_LAST_MATCH;
      YY_CURRENT_BUFFER_LVALUE->yy_buffer_status = YY_BUFFER_EOF_PENDING;
    }
  } else
    ret_val = EOB_ACT_CONTINUE_SCAN;
  if (((yy_n_chars) + number_to_move) > YY_CURRENT_BUFFER_LVALUE->yy_buf_size) {
    int new_size = (yy_n_chars) + number_to_move + ((yy_n_chars) >> 1);
    YY_CURRENT_BUFFER_LVALUE->yy_ch_buf = (char *)yyrealloc(
        (void *)YY_CURRENT_BUFFER_LVALUE->yy_ch_buf, (yy_size_t)new_size);
    if (!YY_CURRENT_BUFFER_LVALUE->yy_ch_buf)
      YY_FATAL_ERROR("out of dynamic memory in yy_get_next_buffer()");
    YY_CURRENT_BUFFER_LVALUE->yy_buf_size = (int)(new_size - 2);
  }
  (yy_n_chars) += number_to_move;
  YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[(yy_n_chars)] = YY_END_OF_BUFFER_CHAR;
  YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[(yy_n_chars) + 1] = YY_END_OF_BUFFER_CHAR;
  (yytext_ptr) = &YY_CURRENT_BUFFER_LVALUE->yy_ch_buf[0];
  return ret_val;
}
static yy_state_type yy_get_previous_state(void) {
  yy_state_type yy_current_state;
  char *yy_cp;
  yy_current_state = (yy_start);
  for (yy_cp = (yytext_ptr) + YY_MORE_ADJ; yy_cp < (yy_c_buf_p); ++yy_cp) {
    YY_CHAR yy_c = (*yy_cp ? yy_ec[YY_SC_TO_UI(*yy_cp)] : 1);
    if (yy_accept[yy_current_state]) {
      (yy_last_accepting_state) = yy_current_state;
      (yy_last_accepting_cpos) = yy_cp;
    }
    while (yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state) {
      yy_current_state = (int)yy_def[yy_current_state];
      if (yy_current_state >= 45)
        yy_c = yy_meta[yy_c];
    }
    yy_current_state = yy_nxt[yy_base[yy_current_state] + yy_c];
  }
  return yy_current_state;
}
static yy_state_type yy_try_NUL_trans(yy_state_type yy_current_state) {
  int yy_is_jam;
  char *yy_cp = (yy_c_buf_p);
  YY_CHAR yy_c = 1;
  if (yy_accept[yy_current_state]) {
    (yy_last_accepting_state) = yy_current_state;
    (yy_last_accepting_cpos) = yy_cp;
  }
  while (yy_chk[yy_base[yy_current_state] + yy_c] != yy_current_state) {
    yy_current_state = (int)yy_def[yy_current_state];
    if (yy_current_state >= 45)
      yy_c = yy_meta[yy_c];
  }
  yy_current_state = yy_nxt[yy_base[yy_current_state] + yy_c];
  yy_is_jam = (yy_current_state == 44);
  return yy_is_jam ? 0 : yy_current_state;
}
#ifndef YY_NO_UNPUT
#endif
void yyrestart(FILE *input_file) {
  if (!YY_CURRENT_BUFFER) {
    yyensure_buffer_stack();
    YY_CURRENT_BUFFER_LVALUE = yy_create_buffer(yyin, YY_BUF_SIZE);
  }
  yy_init_buffer(YY_CURRENT_BUFFER, input_file);
  yy_load_buffer_state();
}
void yy_switch_to_buffer(YY_BUFFER_STATE new_buffer) {
  yyensure_buffer_stack();
  if (YY_CURRENT_BUFFER == new_buffer)
    return;
  if (YY_CURRENT_BUFFER) {
    *(yy_c_buf_p) = (yy_hold_char);
    YY_CURRENT_BUFFER_LVALUE->yy_buf_pos = (yy_c_buf_p);
    YY_CURRENT_BUFFER_LVALUE->yy_n_chars = (yy_n_chars);
  }
  YY_CURRENT_BUFFER_LVALUE = new_buffer;
  yy_load_buffer_state();
  (yy_did_buffer_switch_on_eof) = 1;
}
static void yy_load_buffer_state(void) {
  (yy_n_chars) = YY_CURRENT_BUFFER_LVALUE->yy_n_chars;
  (yytext_ptr) = (yy_c_buf_p) = YY_CURRENT_BUFFER_LVALUE->yy_buf_pos;
  yyin = YY_CURRENT_BUFFER_LVALUE->yy_input_file;
  (yy_hold_char) = *(yy_c_buf_p);
}
YY_BUFFER_STATE yy_create_buffer(FILE *file, int size) {
  YY_BUFFER_STATE b;
  b = (YY_BUFFER_STATE)yyalloc(sizeof(struct yy_buffer_state));
  if (!b)
    YY_FATAL_ERROR("out of dynamic memory in yy_create_buffer()");
  b->yy_buf_size = size;
  b->yy_ch_buf = (char *)yyalloc((yy_size_t)(b->yy_buf_size + 2));
  if (!b->yy_ch_buf)
    YY_FATAL_ERROR("out of dynamic memory in yy_create_buffer()");
  b->yy_is_our_buffer = 1;
  yy_init_buffer(b, file);
  return b;
}
void yy_delete_buffer(YY_BUFFER_STATE b) {
  if (!b)
    return;
  if (b == YY_CURRENT_BUFFER)
    YY_CURRENT_BUFFER_LVALUE = (YY_BUFFER_STATE)0;
  if (b->yy_is_our_buffer)
    yyfree((void *)b->yy_ch_buf);
  yyfree((void *)b);
}
static void yy_init_buffer(YY_BUFFER_STATE b, FILE *file) {
  int oerrno = errno;
  yy_flush_buffer(b);
  b->yy_input_file = file;
  b->yy_fill_buffer = 1;
  if (b != YY_CURRENT_BUFFER) {
    b->yy_bs_lineno = 1;
    b->yy_bs_column = 0;
  }
  b->yy_is_interactive = file ? (isatty(fileno(file)) > 0) : 0;
  errno = oerrno;
}
void yy_flush_buffer(YY_BUFFER_STATE b) {
  if (!b)
    return;
  b->yy_n_chars = 0;
  b->yy_ch_buf[0] = YY_END_OF_BUFFER_CHAR;
  b->yy_ch_buf[1] = YY_END_OF_BUFFER_CHAR;
  b->yy_buf_pos = &b->yy_ch_buf[0];
  b->yy_at_bol = 1;
  b->yy_buffer_status = YY_BUFFER_NEW;
  if (b == YY_CURRENT_BUFFER)
    yy_load_buffer_state();
}
void yypush_buffer_state(YY_BUFFER_STATE new_buffer) {
  if (new_buffer == NULL)
    return;
  yyensure_buffer_stack();
  if (YY_CURRENT_BUFFER) {
    *(yy_c_buf_p) = (yy_hold_char);
    YY_CURRENT_BUFFER_LVALUE->yy_buf_pos = (yy_c_buf_p);
    YY_CURRENT_BUFFER_LVALUE->yy_n_chars = (yy_n_chars);
  }
  if (YY_CURRENT_BUFFER)
    (yy_buffer_stack_top)++;
  YY_CURRENT_BUFFER_LVALUE = new_buffer;
  yy_load_buffer_state();
  (yy_did_buffer_switch_on_eof) = 1;
}
void yypop_buffer_state(void) {
  if (!YY_CURRENT_BUFFER)
    return;
  yy_delete_buffer(YY_CURRENT_BUFFER);
  YY_CURRENT_BUFFER_LVALUE = NULL;
  if ((yy_buffer_stack_top) > 0)
    --(yy_buffer_stack_top);
  if (YY_CURRENT_BUFFER) {
    yy_load_buffer_state();
    (yy_did_buffer_switch_on_eof) = 1;
  }
}
static void yyensure_buffer_stack(void) {
  yy_size_t num_to_alloc;
  if (!(yy_buffer_stack)) {
    num_to_alloc = 1;
    (yy_buffer_stack) = (struct yy_buffer_state **)yyalloc(
        num_to_alloc * sizeof(struct yy_buffer_state *));
    if (!(yy_buffer_stack))
      YY_FATAL_ERROR("out of dynamic memory in yyensure_buffer_stack()");
    memset((yy_buffer_stack), 0,
           num_to_alloc * sizeof(struct yy_buffer_state *));
    (yy_buffer_stack_max) = num_to_alloc;
    (yy_buffer_stack_top) = 0;
    return;
  }
  if ((yy_buffer_stack_top) >= ((yy_buffer_stack_max)) - 1) {
    yy_size_t grow_size = 8;
    num_to_alloc = (yy_buffer_stack_max) + grow_size;
    (yy_buffer_stack) = (struct yy_buffer_state **)yyrealloc(
        (yy_buffer_stack), num_to_alloc * sizeof(struct yy_buffer_state *));
    if (!(yy_buffer_stack))
      YY_FATAL_ERROR("out of dynamic memory in yyensure_buffer_stack()");
    memset((yy_buffer_stack) + (yy_buffer_stack_max), 0,
           grow_size * sizeof(struct yy_buffer_state *));
    (yy_buffer_stack_max) = num_to_alloc;
  }
}
YY_BUFFER_STATE yy_scan_buffer(char *base, yy_size_t size) {
  YY_BUFFER_STATE b;
  if (size < 2 || base[size - 2] != YY_END_OF_BUFFER_CHAR ||
      base[size - 1] != YY_END_OF_BUFFER_CHAR)
    return NULL;
  b = (YY_BUFFER_STATE)yyalloc(sizeof(struct yy_buffer_state));
  if (!b)
    YY_FATAL_ERROR("out of dynamic memory in yy_scan_buffer()");
  b->yy_buf_size = (int)(size - 2);
  b->yy_buf_pos = b->yy_ch_buf = base;
  b->yy_is_our_buffer = 0;
  b->yy_input_file = NULL;
  b->yy_n_chars = b->yy_buf_size;
  b->yy_is_interactive = 0;
  b->yy_at_bol = 1;
  b->yy_fill_buffer = 0;
  b->yy_buffer_status = YY_BUFFER_NEW;
  yy_switch_to_buffer(b);
  return b;
}
YY_BUFFER_STATE yy_scan_string(const char *yystr) {
  return yy_scan_bytes(yystr, (int)strlen(yystr));
}
YY_BUFFER_STATE yy_scan_bytes(const char *yybytes, int _yybytes_len) {
  YY_BUFFER_STATE b;
  char *buf;
  yy_size_t n;
  int i;
  n = (yy_size_t)(_yybytes_len + 2);
  buf = (char *)yyalloc(n);
  if (!buf)
    YY_FATAL_ERROR("out of dynamic memory in yy_scan_bytes()");
  for (i = 0; i < _yybytes_len; ++i)
    buf[i] = yybytes[i];
  buf[_yybytes_len] = buf[_yybytes_len + 1] = YY_END_OF_BUFFER_CHAR;
  b = yy_scan_buffer(buf, n);
  if (!b)
    YY_FATAL_ERROR("bad buffer in yy_scan_bytes()");
  b->yy_is_our_buffer = 1;
  return b;
}
#ifndef YY_EXIT_FAILURE
#define YY_EXIT_FAILURE 2
#endif
static void yynoreturn yy_fatal_error(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(YY_EXIT_FAILURE);
}
#undef yyless
#define yyless(n)                                                              \
  do {                                                                         \
    int yyless_macro_arg = (n);                                                \
    YY_LESS_LINENO(yyless_macro_arg);                                          \
    yytext[yyleng] = (yy_hold_char);                                           \
    (yy_c_buf_p) = yytext + yyless_macro_arg;                                  \
    (yy_hold_char) = *(yy_c_buf_p);                                            \
    *(yy_c_buf_p) = '\0';                                                      \
    yyleng = yyless_macro_arg;                                                 \
  } while (0)
int yyget_lineno(void) { return yylineno; }
FILE *yyget_in(void) { return yyin; }
FILE *yyget_out(void) { return yyout; }
int yyget_leng(void) { return yyleng; }
char *yyget_text(void) { return yytext; }
void yyset_lineno(int _line_number) { yylineno = _line_number; }
void yyset_in(FILE *_in_str) { yyin = _in_str; }
void yyset_out(FILE *_out_str) { yyout = _out_str; }
int yyget_debug(void) { return yy_flex_debug; }
void yyset_debug(int _bdebug) { yy_flex_debug = _bdebug; }
static int yy_init_globals(void) {
  (yy_buffer_stack) = NULL;
  (yy_buffer_stack_top) = 0;
  (yy_buffer_stack_max) = 0;
  (yy_c_buf_p) = NULL;
  (yy_init) = 0;
  (yy_start) = 0;
#ifdef YY_STDINIT
  yyin = stdin;
  yyout = stdout;
#else
  yyin = NULL;
  yyout = NULL;
#endif
  return 0;
}
int yylex_destroy(void) {
  while (YY_CURRENT_BUFFER) {
    yy_delete_buffer(YY_CURRENT_BUFFER);
    YY_CURRENT_BUFFER_LVALUE = NULL;
    yypop_buffer_state();
  }
  yyfree((yy_buffer_stack));
  (yy_buffer_stack) = NULL;
  yy_init_globals();
  return 0;
}
#ifndef yytext_ptr
static void yy_flex_strncpy(char *s1, const char *s2, int n) {
  int i;
  for (i = 0; i < n; ++i)
    s1[i] = s2[i];
}
#endif
#ifdef YY_NEED_STRLEN
static int yy_flex_strlen(const char *s) {
  int n;
  for (n = 0; s[n]; ++n)
    ;
  return n;
}
#endif
void *yyalloc(yy_size_t size) { return malloc(size); }
void *yyrealloc(void *ptr, yy_size_t size) { return realloc(ptr, size); }
void yyfree(void *ptr) { free((char *)ptr); }
#define YYTABLES_NAME "yytables"
#line 68 "temp.l"
void scan_string(const char *str) { yy_switch_to_buffer(yy_scan_string(str)); }
