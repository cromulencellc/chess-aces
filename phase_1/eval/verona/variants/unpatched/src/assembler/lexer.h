#ifndef lexer_HEADER
#define lexer_HEADER
#include "container/list.h"
#include <stdint.h>
enum token_type {
  TOK_INVALID,
  TOK_ADD,
  TOK_SUB,
  TOK_MUL,
  TOK_UDIV,
  TOK_UMOD,
  TOK_SDIV,
  TOK_SMOD,
  TOK_AND,
  TOK_OR,
  TOK_XOR,
  TOK_SHL,
  TOK_SHR,
  TOK_ASR,
  TOK_CMP,
  TOK_MOV,
  TOK_CALL,
  TOK_RET,
  TOK_JMP,
  TOK_JE,
  TOK_JL,
  TOK_JLE,
  TOK_JB,
  TOK_JBE,
  TOK_PUSH,
  TOK_POP,
  TOK_STORE,
  TOK_STOREB,
  TOK_LOAD,
  TOK_LOADB,
  TOK_SYSCALL,
  TOK_R0,
  TOK_R1,
  TOK_R2,
  TOK_R3,
  TOK_R4,
  TOK_R5,
  TOK_R6,
  TOK_R7,
  TOK_SP,
  TOK_COLON,
  TOK_COMMA,
  TOK_BRACKET_OPEN,
  TOK_BRACKET_CLOSE,
  TOK_IDENTIFIER,
  TOK_IMMEDIATE,
  TOK_NEWLINE,
  TOK_DB,
  TOK_DW,
  TOK_DOT_DATA,
  TOK_DOT_CODE,
  TOK_STRING
};
extern const char *token_descriptions[];
struct token {
  enum token_type type;
  char *text;
  int16_t imm;
  unsigned int line_number;
};
void token_delete(struct token *);
struct token *token_copy(const struct token *token);
int lexer(const char *text, struct list **tokens);
#endif