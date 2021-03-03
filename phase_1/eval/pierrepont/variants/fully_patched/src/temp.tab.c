
#define YYBISON 1
#define YYBISON_VERSION "3.0.4"
#define YYSKELETON_NAME "yacc.c"
#define YYPURE 0
#define YYPUSH 0
#define YYPULL 1
#line 1 "temp.y"
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "matrix.h"
#include "matrix_funcs.h"
#include "matrix_vars.h"
#include "testbed.h"
extern int yylex();
extern int yyparse();
extern FILE *yyin;
FILE *file_out = NULL;
int PORT = 3004;
void yyerror(const char *s);
extern void scan_string(const char *);
#line 98 "temp.tab.c"
#ifndef YY_NULLPTR
#if defined __cplusplus && 201103L <= __cplusplus
#define YY_NULLPTR nullptr
#else
#define YY_NULLPTR 0
#endif
#endif
#ifdef YYERROR_VERBOSE
#undef YYERROR_VERBOSE
#define YYERROR_VERBOSE 1
#else
#define YYERROR_VERBOSE 0
#endif
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
#line 166 "temp.tab.c"
};
typedef union YYSTYPE YYSTYPE;
#define YYSTYPE_IS_TRIVIAL 1
#define YYSTYPE_IS_DECLARED 1
#endif
extern YYSTYPE yylval;
int yyparse(void);
#endif
#line 183 "temp.tab.c"
#ifdef short
#undef short
#endif
#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif
#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif
#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif
#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif
#ifndef YYSIZE_T
#ifdef __SIZE_TYPE__
#define YYSIZE_T __SIZE_TYPE__
#elif defined size_t
#define YYSIZE_T size_t
#elif !defined YYSIZE_T
#include <stddef.h>
#define YYSIZE_T size_t
#else
#define YYSIZE_T unsigned int
#endif
#endif
#define YYSIZE_MAXIMUM ((YYSIZE_T)-1)
#ifndef YY_
#if defined YYENABLE_NLS && YYENABLE_NLS
#if ENABLE_NLS
#include <libintl.h>
#define YY_(Msgid) dgettext("bison-runtime", Msgid)
#endif
#endif
#ifndef YY_
#define YY_(Msgid) Msgid
#endif
#endif
#ifndef YY_ATTRIBUTE
#if (defined __GNUC__ &&                                                       \
     (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__))) ||             \
    defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#define YY_ATTRIBUTE(Spec) __attribute__(Spec)
#else
#define YY_ATTRIBUTE(Spec)
#endif
#endif
#ifndef YY_ATTRIBUTE_PURE
#define YY_ATTRIBUTE_PURE YY_ATTRIBUTE((__pure__))
#endif
#ifndef YY_ATTRIBUTE_UNUSED
#define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE((__unused__))
#endif
#if !defined _Noreturn &&                                                      \
    (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
#if defined _MSC_VER && 1200 <= _MSC_VER
#define _Noreturn __declspec(noreturn)
#else
#define _Noreturn YY_ATTRIBUTE((__noreturn__))
#endif
#endif
#if !defined lint || defined __GNUC__
#define YYUSE(E) ((void)(E))
#else
#define YYUSE(E)
#endif
#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
#define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                                    \
  _Pragma("GCC diagnostic push")                                               \
      _Pragma("GCC diagnostic ignored \"-Wuninitialized\"")                    \
          _Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
#define YY_IGNORE_MAYBE_UNINITIALIZED_END _Pragma("GCC diagnostic pop")
#else
#define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
#define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
#define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
#define YY_INITIAL_VALUE(Value)
#endif
#if !defined yyoverflow || YYERROR_VERBOSE
#ifdef YYSTACK_USE_ALLOCA
#if YYSTACK_USE_ALLOCA
#ifdef __GNUC__
#define YYSTACK_ALLOC __builtin_alloca
#elif defined __BUILTIN_VA_ARG_INCR
#include <alloca.h>
#elif defined _AIX
#define YYSTACK_ALLOC __alloca
#elif defined _MSC_VER
#include <malloc.h>
#define alloca _alloca
#else
#define YYSTACK_ALLOC alloca
#if !defined _ALLOCA_H && !defined EXIT_SUCCESS
#include <stdlib.h>
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#endif
#endif
#endif
#endif
#ifdef YYSTACK_ALLOC
#define YYSTACK_FREE(Ptr)                                                      \
  do {                                                                         \
    ;                                                                          \
  } while (0)
#ifndef YYSTACK_ALLOC_MAXIMUM
#define YYSTACK_ALLOC_MAXIMUM 4032
#endif
#else
#define YYSTACK_ALLOC YYMALLOC
#define YYSTACK_FREE YYFREE
#ifndef YYSTACK_ALLOC_MAXIMUM
#define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#endif
#if (defined __cplusplus && !defined EXIT_SUCCESS &&                           \
     !((defined YYMALLOC || defined malloc) &&                                 \
       (defined YYFREE || defined free)))
#include <stdlib.h>
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#endif
#ifndef YYMALLOC
#define YYMALLOC malloc
#if !defined malloc && !defined EXIT_SUCCESS
void *malloc(YYSIZE_T);
#endif
#endif
#ifndef YYFREE
#define YYFREE free
#if !defined free && !defined EXIT_SUCCESS
void free(void *);
#endif
#endif
#endif
#endif
#if (!defined yyoverflow &&                                                    \
     (!defined __cplusplus ||                                                  \
      (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))
union yyalloc {
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};
#define YYSTACK_GAP_MAXIMUM (sizeof(union yyalloc) - 1)
#define YYSTACK_BYTES(N)                                                       \
  ((N) * (sizeof(yytype_int16) + sizeof(YYSTYPE)) + YYSTACK_GAP_MAXIMUM)
#define YYCOPY_NEEDED 1
#define YYSTACK_RELOCATE(Stack_alloc, Stack)                                   \
  do {                                                                         \
    YYSIZE_T yynewbytes;                                                       \
    YYCOPY(&yyptr->Stack_alloc, Stack, yysize);                                \
    Stack = &yyptr->Stack_alloc;                                               \
    yynewbytes = yystacksize * sizeof(*Stack) + YYSTACK_GAP_MAXIMUM;           \
    yyptr += yynewbytes / sizeof(*yyptr);                                      \
  } while (0)
#endif
#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
#ifndef YYCOPY
#if defined __GNUC__ && 1 < __GNUC__
#define YYCOPY(Dst, Src, Count)                                                \
  __builtin_memcpy(Dst, Src, (Count) * sizeof(*(Src)))
#else
#define YYCOPY(Dst, Src, Count)                                                \
  do {                                                                         \
    YYSIZE_T yyi;                                                              \
    for (yyi = 0; yyi < (Count); yyi++)                                        \
      (Dst)[yyi] = (Src)[yyi];                                                 \
  } while (0)
#endif
#endif
#endif
#define YYFINAL 2
#define YYLAST 153
#define YYNTOKENS 22
#define YYNNTS 4
#define YYNRULES 21
#define YYNSTATES 43
#define YYUNDEFTOK 2
#define YYMAXUTOK 276
#define YYTRANSLATE(YYX)                                                       \
  ((unsigned int)(YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)
static const yytype_uint8 yytranslate[] = {
    0, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 2, 1, 2, 3, 4, 5, 6, 7, 8,
    9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21};
#if YYDEBUG
static const yytype_uint8 yyrline[] = {0,  55, 55, 56, 59, 60, 67, 68,
                                       77, 78, 79, 80, 81, 82, 83, 84,
                                       85, 86, 87, 88, 89, 90};
#endif
#if YYDEBUG || YYERROR_VERBOSE || 0
static const char *const yytname[] = {
    "$end",         "error",      "$undefined",   "NAME",        "PLUS",
    "MINUS",        "MULTIPLY",   "LPAREN",       "RPAREN",      "ASSIGN",
    "SINGLE_QUOTE", "NEWLINE",    "QUIT",         "COMMA",       "HAT",
    "DOT_HAT",      "DOT_DIVIDE", "DOT_MULTIPLY", "BARS",        "MATRIX",
    "FUNCTION",     "DIVIDE",     "$accept",      "calculation", "line",
    "matrix_math",  YY_NULLPTR};
#endif
#ifdef YYPRINT
static const yytype_uint16 yytoknum[] = {0,   256, 257, 258, 259, 260, 261, 262,
                                         263, 264, 265, 266, 267, 268, 269, 270,
                                         271, 272, 273, 274, 275, 276};
#endif
#define YYPACT_NINF -8
#define yypact_value_is_default(Yystate) (!!((Yystate) == (-8)))
#define YYTABLE_NINF -1
#define yytable_value_is_error(Yytable_value) 0
static const yytype_int16 yypact[] = {
    -8, 23, -8, -7,  26,  -8,  6,   26,  -8, -3, -8, 72,  26,  -8, 86,
    -8, 43, 26, 26,  26,  26,  -8,  -8,  26, 26, 26, 26,  100, -8, -8,
    58, -5, -5, 136, 128, 128, 128, 128, -8, -8, 26, 114, -8};
static const yytype_uint8 yydefact[] = {
    2, 0, 1, 21, 0, 4, 0,  0,  8, 0, 3,  0,  0,  21, 0,  6,  0, 0,  0, 0, 0, 16,
    5, 0, 0, 0,  0, 0, 20, 19, 0, 9, 12, 15, 10, 11, 14, 13, 7, 17, 0, 0, 18};
static const yytype_int8 yypgoto[] = {-8, -8, -8, -4};
static const yytype_int8 yydefgoto[] = {-1, 1, 10, 11};
static const yytype_uint8 yytable[] = {
    14, 20, 12, 16, 17, 21, 0,  0,  27, 23, 24, 25, 26, 30, 31, 32, 33, 15,
    0,  34, 35, 36, 37, 2,  0,  0,  3,  0,  0,  13, 4,  0,  0,  4,  5,  6,
    41, 0,  0,  0,  0,  7,  8,  9,  7,  8,  9,  18, 19, 20, 0,  0,  0,  21,
    0,  0,  0,  23, 24, 25, 26, 29, 18, 19, 20, 0,  39, 0,  21, 0,  0,  40,
    23, 24, 25, 26, 18, 19, 20, 0,  0,  0,  21, 22, 0,  0,  23, 24, 25, 26,
    18, 19, 20, 0,  28, 0,  21, 0,  0,  0,  23, 24, 25, 26, 18, 19, 20, 0,
    0,  0,  21, 38, 0,  0,  23, 24, 25, 26, 18, 19, 20, 0,  42, 0,  21, 0,
    0,  0,  23, 24, 25, 26, 18, 19, 20, 0,  0,  0,  21, 0,  0,  0,  23, 24,
    25, 26, 21, 0,  0,  0,  23, 24, 25, 26};
static const yytype_int8 yycheck[] = {
    4,  6,  9,  7,  7,  10, -1, -1, 12, 14, 15, 16, 17, 17, 18, 19, 20, 11,
    -1, 23, 24, 25, 26, 0,  -1, -1, 3,  -1, -1, 3,  7,  -1, -1, 7,  11, 12,
    40, -1, -1, -1, -1, 18, 19, 20, 18, 19, 20, 4,  5,  6,  -1, -1, -1, 10,
    -1, -1, -1, 14, 15, 16, 17, 18, 4,  5,  6,  -1, 8,  -1, 10, -1, -1, 13,
    14, 15, 16, 17, 4,  5,  6,  -1, -1, -1, 10, 11, -1, -1, 14, 15, 16, 17,
    4,  5,  6,  -1, 8,  -1, 10, -1, -1, -1, 14, 15, 16, 17, 4,  5,  6,  -1,
    -1, -1, 10, 11, -1, -1, 14, 15, 16, 17, 4,  5,  6,  -1, 8,  -1, 10, -1,
    -1, -1, 14, 15, 16, 17, 4,  5,  6,  -1, -1, -1, 10, -1, -1, -1, 14, 15,
    16, 17, 10, -1, -1, -1, 14, 15, 16, 17};
static const yytype_uint8 yystos[] = {
    0,  23, 0,  3,  7,  11, 12, 18, 19, 20, 24, 25, 9,  3, 25,
    11, 25, 7,  4,  5,  6,  10, 11, 14, 15, 16, 17, 25, 8, 18,
    25, 25, 25, 25, 25, 25, 25, 25, 11, 8,  13, 25, 8};
static const yytype_uint8 yyr1[] = {0,  22, 23, 23, 24, 24, 24, 24, 25, 25, 25,
                                    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25};
static const yytype_uint8 yyr2[] = {0, 2, 0, 2, 1, 2, 2, 4, 1, 3, 3,
                                    3, 3, 3, 3, 3, 2, 4, 6, 3, 3, 1};
#define yyerrok (yyerrstatus = 0)
#define yyclearin (yychar = YYEMPTY)
#define YYEMPTY (-2)
#define YYEOF 0
#define YYACCEPT goto yyacceptlab
#define YYABORT goto yyabortlab
#define YYERROR goto yyerrorlab
#define YYRECOVERING() (!!yyerrstatus)
#define YYBACKUP(Token, Value)                                                 \
  do                                                                           \
    if (yychar == YYEMPTY) {                                                   \
      yychar = (Token);                                                        \
      yylval = (Value);                                                        \
      YYPOPSTACK(yylen);                                                       \
      yystate = *yyssp;                                                        \
      goto yybackup;                                                           \
    } else {                                                                   \
      yyerror(YY_("syntax error: cannot back up"));                            \
      YYERROR;                                                                 \
    }                                                                          \
  while (0)
#define YYTERROR 1
#define YYERRCODE 256
#if YYDEBUG
#ifndef YYFPRINTF
#include <stdio.h>
#define YYFPRINTF fprintf
#endif
#define YYDPRINTF(Args)                                                        \
  do {                                                                         \
    if (yydebug)                                                               \
      YYFPRINTF Args;                                                          \
  } while (0)
#ifndef YY_LOCATION_PRINT
#define YY_LOCATION_PRINT(File, Loc) ((void)0)
#endif
#define YY_SYMBOL_PRINT(Title, Type, Value, Location)                          \
  do {                                                                         \
    if (yydebug) {                                                             \
      YYFPRINTF(stderr, "%s ", Title);                                         \
      yy_symbol_print(stderr, Type, Value);                                    \
      YYFPRINTF(stderr, "\n");                                                 \
    }                                                                          \
  } while (0)
static void yy_symbol_value_print(FILE *yyoutput, int yytype,
                                  YYSTYPE const *const yyvaluep) {
  FILE *yyo = yyoutput;
  YYUSE(yyo);
  if (!yyvaluep)
    return;
#ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT(yyoutput, yytoknum[yytype], *yyvaluep);
#endif
  YYUSE(yytype);
}
static void yy_symbol_print(FILE *yyoutput, int yytype,
                            YYSTYPE const *const yyvaluep) {
  YYFPRINTF(yyoutput, "%s %s (", yytype < YYNTOKENS ? "token" : "nterm",
            yytname[yytype]);
  yy_symbol_value_print(yyoutput, yytype, yyvaluep);
  YYFPRINTF(yyoutput, ")");
}
static void yy_stack_print(yytype_int16 *yybottom, yytype_int16 *yytop) {
  YYFPRINTF(stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++) {
    int yybot = *yybottom;
    YYFPRINTF(stderr, " %d", yybot);
  }
  YYFPRINTF(stderr, "\n");
}
#define YY_STACK_PRINT(Bottom, Top)                                            \
  do {                                                                         \
    if (yydebug)                                                               \
      yy_stack_print((Bottom), (Top));                                         \
  } while (0)
static void yy_reduce_print(yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule) {
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF(stderr, "Reducing stack by rule %d (line %lu):\n", yyrule - 1,
            yylno);
  for (yyi = 0; yyi < yynrhs; yyi++) {
    YYFPRINTF(stderr, "   $%d = ", yyi + 1);
    yy_symbol_print(stderr, yystos[yyssp[yyi + 1 - yynrhs]],
                    &(yyvsp[(yyi + 1) - (yynrhs)]));
    YYFPRINTF(stderr, "\n");
  }
}
#define YY_REDUCE_PRINT(Rule)                                                  \
  do {                                                                         \
    if (yydebug)                                                               \
      yy_reduce_print(yyssp, yyvsp, Rule);                                     \
  } while (0)
int yydebug;
#else
#define YYDPRINTF(Args)
#define YY_SYMBOL_PRINT(Title, Type, Value, Location)
#define YY_STACK_PRINT(Bottom, Top)
#define YY_REDUCE_PRINT(Rule)
#endif
#ifndef YYINITDEPTH
#define YYINITDEPTH 200
#endif
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif
#if YYERROR_VERBOSE
#ifndef yystrlen
#if defined __GLIBC__ && defined _STRING_H
#define yystrlen strlen
#else
static YYSIZE_T yystrlen(const char *yystr) {
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#endif
#endif
#ifndef yystpcpy
#if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#define yystpcpy stpcpy
#else
static char *yystpcpy(char *yydest, const char *yysrc) {
  char *yyd = yydest;
  const char *yys = yysrc;
  while ((*yyd++ = *yys++) != '\0')
    continue;
  return yyd - 1;
}
#endif
#endif
#ifndef yytnamerr
static YYSIZE_T yytnamerr(char *yyres, const char *yystr) {
  if (*yystr == '"') {
    YYSIZE_T yyn = 0;
    char const *yyp = yystr;
    for (;;)
      switch (*++yyp) {
      case '\'':
      case ',':
        goto do_not_strip_quotes;
      case '\\':
        if (*++yyp != '\\')
          goto do_not_strip_quotes;
      default:
        if (yyres)
          yyres[yyn] = *yyp;
        yyn++;
        break;
      case '"':
        if (yyres)
          yyres[yyn] = '\0';
        return yyn;
      }
  do_not_strip_quotes:;
  }
  if (!yyres)
    return yystrlen(yystr);
  return yystpcpy(yyres, yystr) - yyres;
}
#endif
static int yysyntax_error(YYSIZE_T *yymsg_alloc, char **yymsg,
                          yytype_int16 *yyssp, int yytoken) {
  YYSIZE_T yysize0 = yytnamerr(YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  const char *yyformat = YY_NULLPTR;
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  int yycount = 0;
  if (yytoken != YYEMPTY) {
    int yyn = yypact[*yyssp];
    yyarg[yycount++] = yytname[yytoken];
    if (!yypact_value_is_default(yyn)) {
      int yyxbegin = yyn < 0 ? -yyn : 0;
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR &&
            !yytable_value_is_error(yytable[yyx + yyn])) {
          if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM) {
            yycount = 1;
            yysize = yysize0;
            break;
          }
          yyarg[yycount++] = yytname[yyx];
          {
            YYSIZE_T yysize1 = yysize + yytnamerr(YY_NULLPTR, yytname[yyx]);
            if (!(yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
              return 2;
            yysize = yysize1;
          }
        }
    }
  }
  switch (yycount) {
#define YYCASE_(N, S)                                                          \
  case N:                                                                      \
    yyformat = S;                                                              \
    break
    YYCASE_(0, YY_("syntax error"));
    YYCASE_(1, YY_("syntax error, unexpected %s"));
    YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
    YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
    YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
    YYCASE_(5,
            YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
  }
  {
    YYSIZE_T yysize1 = yysize + yystrlen(yyformat);
    if (!(yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }
  if (*yymsg_alloc < yysize) {
    *yymsg_alloc = 2 * yysize;
    if (!(yysize <= *yymsg_alloc && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
      *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
    return 1;
  }
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount) {
        yyp += yytnamerr(yyp, yyarg[yyi++]);
        yyformat += 2;
      } else {
        yyp++;
        yyformat++;
      }
  }
  return 0;
}
#endif
static void yydestruct(const char *yymsg, int yytype, YYSTYPE *yyvaluep) {
  YYUSE(yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT(yymsg, yytype, yyvaluep, yylocationp);
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE(yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}
int yychar;
YYSTYPE yylval;
int yynerrs;
int yyparse(void) {
  int yystate;
  int yyerrstatus;
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss;
  yytype_int16 *yyssp;
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs;
  YYSTYPE *yyvsp;
  YYSIZE_T yystacksize;
  int yyn;
  int yyresult;
  int yytoken = 0;
  YYSTYPE yyval;
#if YYERROR_VERBOSE
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif
#define YYPOPSTACK(N) (yyvsp -= (N), yyssp -= (N))
  int yylen = 0;
  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;
  YYDPRINTF((stderr, "Starting parse\n"));
  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;
  goto yysetstate;
yynewstate:
  yyssp++;
yysetstate:
  *yyssp = yystate;
  if (yyss + yystacksize - 1 <= yyssp) {
    YYSIZE_T yysize = yyssp - yyss + 1;
#ifdef yyoverflow
    {
      YYSTYPE *yyvs1 = yyvs;
      yytype_int16 *yyss1 = yyss;
      yyoverflow(YY_("memory exhausted"), &yyss1, yysize * sizeof(*yyssp),
                 &yyvs1, yysize * sizeof(*yyvsp), &yystacksize);
      yyss = yyss1;
      yyvs = yyvs1;
    }
#else
#ifndef YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    if (YYMAXDEPTH <= yystacksize)
      goto yyexhaustedlab;
    yystacksize *= 2;
    if (YYMAXDEPTH < yystacksize)
      yystacksize = YYMAXDEPTH;
    {
      yytype_int16 *yyss1 = yyss;
      union yyalloc *yyptr =
          (union yyalloc *)YYSTACK_ALLOC(YYSTACK_BYTES(yystacksize));
      if (!yyptr)
        goto yyexhaustedlab;
      YYSTACK_RELOCATE(yyss_alloc, yyss);
      YYSTACK_RELOCATE(yyvs_alloc, yyvs);
#undef YYSTACK_RELOCATE
      if (yyss1 != yyssa)
        YYSTACK_FREE(yyss1);
    }
#endif
#endif
    yyssp = yyss + yysize - 1;
    yyvsp = yyvs + yysize - 1;
    YYDPRINTF((stderr, "Stack size increased to %lu\n",
               (unsigned long int)yystacksize));
    if (yyss + yystacksize - 1 <= yyssp)
      YYABORT;
  }
  YYDPRINTF((stderr, "Entering state %d\n", yystate));
  if (yystate == YYFINAL)
    YYACCEPT;
  goto yybackup;
yybackup:
  yyn = yypact[yystate];
  if (yypact_value_is_default(yyn))
    goto yydefault;
  if (yychar == YYEMPTY) {
    YYDPRINTF((stderr, "Reading a token: "));
    yychar = yylex();
  }
  if (yychar <= YYEOF) {
    yychar = yytoken = YYEOF;
    YYDPRINTF((stderr, "Now at end of input.\n"));
  } else {
    yytoken = YYTRANSLATE(yychar);
    YY_SYMBOL_PRINT("Next token is", yytoken, &yylval, &yylloc);
  }
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0) {
    if (yytable_value_is_error(yyn))
      goto yyerrlab;
    yyn = -yyn;
    goto yyreduce;
  }
  if (yyerrstatus)
    yyerrstatus--;
  YY_SYMBOL_PRINT("Shifting", yytoken, &yylval, &yylloc);
  yychar = YYEMPTY;
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  goto yynewstate;
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;
yyreduce:
  yylen = yyr2[yyn];
  yyval = yyvsp[1 - yylen];
  YY_REDUCE_PRINT(yyn);
  switch (yyn) {
  case 5:
#line 60 "temp.y"
  {
    if ((yyvsp[-1].m) != NULL) {
      print_matrix((yyvsp[-1].m));
      add_var("ans", (yyvsp[-1].m));
    }
  }
#line 1308 "temp.tab.c"
  break;
  case 6:
#line 67 "temp.y"
  {
    fprintf(file_out, "bye!\n");
    exit(0);
  }
#line 1314 "temp.tab.c"
  break;
  case 7:
#line 68 "temp.y"
  {
    if ((yyvsp[-1].m) != NULL) {
      add_var((yyvsp[-3].var), (yyvsp[-1].m));
      fprintf(file_out, "%s = \n", (yyvsp[-3].var));
      print_matrix((yyvsp[-1].m));
    }
  }
#line 1326 "temp.tab.c"
  break;
  case 8:
#line 77 "temp.y"
  {
    (yyval.m) = (yyvsp[0].m);
  }
#line 1332 "temp.tab.c"
  break;
  case 9:
#line 78 "temp.y"
  {
    (yyval.m) = add_matrices((yyvsp[-2].m), (yyvsp[0].m));
  }
#line 1338 "temp.tab.c"
  break;
  case 10:
#line 79 "temp.y"
  {
    (yyval.m) = exp_matrix((yyvsp[-2].m), (yyvsp[0].m));
  }
#line 1344 "temp.tab.c"
  break;
  case 11:
#line 80 "temp.y"
  {
    (yyval.m) = exp_matrix_elw((yyvsp[-2].m), (yyvsp[0].m));
  }
#line 1350 "temp.tab.c"
  break;
  case 12:
#line 81 "temp.y"
  {
    (yyval.m) = sub_matrices((yyvsp[-2].m), (yyvsp[0].m));
  }
#line 1356 "temp.tab.c"
  break;
  case 13:
#line 82 "temp.y"
  {
    (yyval.m) = dot_multiply_matrices((yyvsp[-2].m), (yyvsp[0].m));
  }
#line 1362 "temp.tab.c"
  break;
  case 14:
#line 83 "temp.y"
  {
    (yyval.m) = dot_divide_matrices((yyvsp[-2].m), (yyvsp[0].m));
  }
#line 1368 "temp.tab.c"
  break;
  case 15:
#line 84 "temp.y"
  {
    (yyval.m) = multiply_matrices((yyvsp[-2].m), (yyvsp[0].m));
  }
#line 1374 "temp.tab.c"
  break;
  case 16:
#line 85 "temp.y"
  {
    (yyval.m) = matrix_transpose((yyvsp[-1].m));
  }
#line 1380 "temp.tab.c"
  break;
  case 17:
#line 86 "temp.y"
  {
    (yyval.m) = handle_func_single_arg((yyvsp[-3].func), (yyvsp[-1].m));
  }
#line 1386 "temp.tab.c"
  break;
  case 18:
#line 87 "temp.y"
  {
    (yyval.m) =
        handle_func_two_arg((yyvsp[-5].func), (yyvsp[-3].m), (yyvsp[-1].m));
  }
#line 1392 "temp.tab.c"
  break;
  case 19:
#line 88 "temp.y"
  {
    (yyval.m) = magnitude((yyvsp[-1].m));
  }
#line 1398 "temp.tab.c"
  break;
  case 20:
#line 89 "temp.y"
  {
    (yyval.m) = (yyvsp[-1].m);
  }
#line 1404 "temp.tab.c"
  break;
  case 21:
#line 90 "temp.y"
  {
    matrix *tempm = get_matrix_copy((yyvsp[0].var));
    if (tempm != NULL) {
      (yyval.m) = tempm;
    } else {
      fprintf(file_out, "error %s -- unknown variable\n", (yyvsp[0].var));
      (yyval.m) = NULL;
    }
  }
#line 1419 "temp.tab.c"
  break;
#line 1423 "temp.tab.c"
  default:
    break;
  }
  YY_SYMBOL_PRINT("-> $$ =", yyr1[yyn], &yyval, &yyloc);
  YYPOPSTACK(yylen);
  yylen = 0;
  YY_STACK_PRINT(yyss, yyssp);
  *++yyvsp = yyval;
  yyn = yyr1[yyn];
  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];
  goto yynewstate;
yyerrlab:
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE(yychar);
  if (!yyerrstatus) {
    ++yynerrs;
#if !YYERROR_VERBOSE
    yyerror(YY_("syntax error"));
#else
#define YYSYNTAX_ERROR yysyntax_error(&yymsg_alloc, &yymsg, yyssp, yytoken)
    {
      char const *yymsgp = YY_("syntax error");
      int yysyntax_error_status;
      yysyntax_error_status = YYSYNTAX_ERROR;
      if (yysyntax_error_status == 0)
        yymsgp = yymsg;
      else if (yysyntax_error_status == 1) {
        if (yymsg != yymsgbuf)
          YYSTACK_FREE(yymsg);
        yymsg = (char *)YYSTACK_ALLOC(yymsg_alloc);
        if (!yymsg) {
          yymsg = yymsgbuf;
          yymsg_alloc = sizeof yymsgbuf;
          yysyntax_error_status = 2;
        } else {
          yysyntax_error_status = YYSYNTAX_ERROR;
          yymsgp = yymsg;
        }
      }
      yyerror(yymsgp);
      if (yysyntax_error_status == 2)
        goto yyexhaustedlab;
    }
#undef YYSYNTAX_ERROR
#endif
  }
  if (yyerrstatus == 3) {
    if (yychar <= YYEOF) {
      if (yychar == YYEOF)
        YYABORT;
    } else {
      yydestruct("Error: discarding", yytoken, &yylval);
      yychar = YYEMPTY;
    }
  }
  goto yyerrlab1;
yyerrorlab:
  if (0)
    goto yyerrorlab;
  YYPOPSTACK(yylen);
  yylen = 0;
  YY_STACK_PRINT(yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;
yyerrlab1:
  yyerrstatus = 3;
  for (;;) {
    yyn = yypact[yystate];
    if (!yypact_value_is_default(yyn)) {
      yyn += YYTERROR;
      if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR) {
        yyn = yytable[yyn];
        if (0 < yyn)
          break;
      }
    }
    if (yyssp == yyss)
      YYABORT;
    yydestruct("Error: popping", yystos[yystate], yyvsp);
    YYPOPSTACK(1);
    yystate = *yyssp;
    YY_STACK_PRINT(yyss, yyssp);
  }
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  YY_SYMBOL_PRINT("Shifting", yystos[yyn], yyvsp, yylsp);
  yystate = yyn;
  goto yynewstate;
yyacceptlab:
  yyresult = 0;
  goto yyreturn;
yyabortlab:
  yyresult = 1;
  goto yyreturn;
#if !defined yyoverflow || YYERROR_VERBOSE
yyexhaustedlab:
  yyerror(YY_("memory exhausted"));
  yyresult = 2;
#endif
yyreturn:
  if (yychar != YYEMPTY) {
    yytoken = YYTRANSLATE(yychar);
    yydestruct("Cleanup: discarding lookahead", yytoken, &yylval);
  }
  YYPOPSTACK(yylen);
  YY_STACK_PRINT(yyss, yyssp);
  while (yyssp != yyss) {
    yydestruct("Cleanup: popping", yystos[*yyssp], yyvsp);
    YYPOPSTACK(1);
  }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE(yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE(yymsg);
#endif
  return yyresult;
}
#line 102 "temp.y"
#define MAXREAD 1024
char *readuntil(int fd, char c) {
  char *line = calloc(1, MAXREAD + 1);
  char t;
  int index = 0;
  if (!line) {
    return NULL;
  }
  while (read(fd, &t, 1) >= 0 && index < MAXREAD) {
    line[index] = t;
    index++;
    if (t == c) {
      return line;
    }
  }
  return line;
}
char *readline(FILE *n) {
  char *lineptr = NULL;
  size_t length = 0;
  if (!n) {
    return NULL;
  }
  getline(&lineptr, &length, n);
  return lineptr;
}
int setup_socket(int port) {
  int fd;
  struct sockaddr_in sa;
  int enable = 1;
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    fprintf(stderr, "socket fail\n");
    exit(0);
  }
  memset(&sa, 0, sizeof(struct sockaddr_in));
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = INADDR_ANY;
  sa.sin_port = htons(port);
  if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
    fprintf(stderr, "Error on binding\n");
    close(fd);
    return -1;
  }
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable,
                 sizeof(enable)) < 0) {
    fprintf(stderr, "Error on setsockopt\n");
    close(fd);
    return -1;
  }
  if (listen(fd, 0) < 0) {
    fprintf(stderr, "Error on listen\n");
    close(fd);
    return -1;
  }
  fprintf(stdout, "[INFO] Listener socket on port: %d\n", port);
  return fd;
}
int accept_conn(int fd) {
  int conn_fd = 0;
  struct sockaddr_in ca;
  socklen_t ca_len = sizeof(struct sockaddr_in);
  memset(&ca, 0, sizeof(struct sockaddr_in));
  conn_fd = accept(fd, (struct sockaddr *)&ca, &ca_len);
  if (conn_fd < 0) {
    fprintf(stderr, "Error accepting client.\n");
    close(fd);
    return -1;
  }
  close(fd);
  return conn_fd;
}
void usage(char *pn) {
  fprintf(stderr, "USAGE: %s -p <port> -s\n", pn);
  fprintf(stderr,
          "The -s option is used to specify a client/server interaction\n");
  fprintf(stderr, "The -p option is optional and defaults to 3004. It can also "
                  "be specified via the PORT environment variable.\n");
  exit(1);
  return;
}
int main(int argc, char **argv, char *envp[]) {
  int server_flag = 0;
  int connfd, fd = 0;
  char c;
#ifndef NO_TESTBED
  assert_execution_on_testbed();
#endif
  int infd = fileno(stdin);
  int outfd = fileno(stdout);
  if (getenv("PORT")) {
    PORT = atoi(getenv("PORT"));
  }
  while ((c = getopt(argc, argv, "p:s")) != -1) {
    switch (c) {
    case 'p':
      PORT = atoi(optarg);
      break;
    case 's':
      server_flag = 1;
      break;
    case '?':
      if (optopt == 'p') {
        fprintf(stderr, "-%c argument required\n", optopt);
        usage(argv[0]);
      } else {
        fprintf(stderr, "Unknown option\n");
        usage(argv[0]);
      }
    default:
      exit(1);
    }
  }
  if (server_flag) {
    fd = setup_socket(PORT);
    connfd = accept_conn(fd);
    if (connfd < 0) {
      exit(0);
    }
    infd = connfd;
    outfd = connfd;
  }
  file_out = fdopen(dup(outfd), "w");
  fprintf(file_out, ">>> ");
  fflush(file_out);
  char *line = readuntil(infd, '\n');
  while (line) {
    scan_string(line);
    yyparse();
    free(line);
    fprintf(file_out, ">>> ");
    fflush(file_out);
    line = readuntil(infd, '\n');
  }
  return 0;
}
void yyerror(const char *s) { fprintf(file_out, "Parse error: %s\n", s); }
