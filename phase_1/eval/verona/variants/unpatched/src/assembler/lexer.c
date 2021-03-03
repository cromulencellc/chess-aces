#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "rust.h"
struct token_matcher {
  enum token_type token;
  const char *string;
};
const char *token_descriptions[] = {
    "TOK_INVALID",    "TOK_ADD",       "TOK_SUB",          "TOK_MUL",
    "TOK_UDIV",       "TOK_UMOD",      "TOK_SDIV",         "TOK_SMOD",
    "TOK_AND",        "TOK_OR",        "TOK_XOR",          "TOK_SHL",
    "TOK_SHR",        "TOK_ASR",       "TOK_CMP",          "TOK_MOV",
    "TOK_CALL",       "TOK_RET",       "TOK_JMP",          "TOK_JE",
    "TOK_JL",         "TOK_JLE",       "TOK_JB",           "TOK_JBE",
    "TOK_PUSH",       "TOK_POP",       "TOK_STORE",        "TOK_STOREB",
    "TOK_LOAD",       "TOK_LOADB",     "TOK_SYSCALL",      "TOK_R0",
    "TOK_R1",         "TOK_R2",        "TOK_R3",           "TOK_R4",
    "TOK_R5",         "TOK_R6",        "TOK_R7",           "TOK_SP",
    "TOK_COLON",      "TOK_COMMA",     "TOK_BRACKET_OPEN", "TOK_BRACKET_CLOSE",
    "TOK_IDENTIFIER", "TOK_IMMEDIATE", "TOK_NEWLINE",      "TOK_DB",
    "TOK_DW",         "TOK_DOT_DATA",  "TOK_DOT_CODE",     "TOK_STRING"};
struct token_matcher token_matchers[] = {
    {TOK_ADD, "add"},         {TOK_SUB, "sub"},       {TOK_MUL, "mul"},
    {TOK_UDIV, "udiv"},       {TOK_UMOD, "umod"},     {TOK_SDIV, "sdiv"},
    {TOK_SMOD, "smod"},       {TOK_AND, "and"},       {TOK_OR, "or"},
    {TOK_XOR, "xor"},         {TOK_SHL, "shl"},       {TOK_SHR, "shr"},
    {TOK_ASR, "asr"},         {TOK_CMP, "cmp"},       {TOK_MOV, "mov"},
    {TOK_CALL, "call"},       {TOK_RET, "ret"},       {TOK_JMP, "jmp"},
    {TOK_JE, "je"},           {TOK_JLE, "jle"},       {TOK_JL, "jl"},
    {TOK_JBE, "jbe"},         {TOK_JB, "jb"},         {TOK_PUSH, "push"},
    {TOK_POP, "pop"},         {TOK_STOREB, "storeb"}, {TOK_STORE, "store"},
    {TOK_LOADB, "loadb"},     {TOK_LOAD, "load"},     {TOK_SYSCALL, "syscall"},
    {TOK_R0, "r0"},           {TOK_R1, "r1"},         {TOK_R2, "r2"},
    {TOK_R3, "r3"},           {TOK_R4, "r4"},         {TOK_R5, "r5"},
    {TOK_R6, "r6"},           {TOK_R7, "r7"},         {TOK_SP, "sp"},
    {TOK_COLON, ":"},         {TOK_COMMA, ","},       {TOK_BRACKET_OPEN, "["},
    {TOK_BRACKET_CLOSE, "]"}, {TOK_DB, "db"},         {TOK_DW, "dw"},
    {TOK_DOT_DATA, ".data"},  {TOK_DOT_CODE, ".code"}};
struct token *token_create(unsigned int line_number, enum token_type type,
                           const char *text, uint32_t text_len) {
  struct token *token = (struct token *)malloc(sizeof(struct token));
  token->line_number = line_number;
  token->type = type;
  token->text = strndup(text, text_len);
  return token;
}
void token_delete(struct token *token) {
  free(token->text);
  free(token);
}
struct token *token_copy(const struct token *token) {
  struct token *t = (struct token *)malloc(sizeof(struct token));
  t->line_number = token->line_number;
  t->type = token->type;
  t->text = strdup(token->text);
  t->imm = token->imm;
  return t;
}
const struct token_matcher *match_token(const char *s) {
  unsigned int i;
  for (i = 0; i < sizeof(token_matchers) / sizeof(struct token_matcher); i++) {
    if (strncmp(token_matchers[i].string, s,
                strlen(token_matchers[i].string)) == 0) {
      return &token_matchers[i];
    }
  }
  return NULL;
}
int lexer(const char *text, struct list **tokens) {
  *tokens = list_create();
  const char *c = text;
  unsigned int line_number = 1;
  while (*c != 0) {
    if ((*c == '\t') || (*c == ' ') || (*c == '\r')) {
      c++;
      continue;
    }
    if (*c == ';') {
      while ((*c != '\0') && (*c != '\n')) {
        c++;
      }
      continue;
    }
    if (*c == '\n') {
      struct token *token = token_create(line_number, TOK_NEWLINE, "\n", 1);
      list_append(*tokens, token);
      line_number++;
      c++;
      continue;
    }
    const struct token_matcher *tm = match_token(c);
    if (tm != NULL) {
      struct token *token =
          token_create(line_number, tm->token, tm->string, strlen(tm->string));
      list_append(*tokens, token);
      c += strlen(tm->string);
      continue;
    }
    if ((*c == '0') && (c[1] == 'x')) {
      char buf[16];
      unsigned int i;
      for (i = 0; i < 15; i++) {
        if (((c[i + 2] >= '0') && (c[i + 2] <= '9')) ||
            ((c[i + 2] >= 'a') && (c[i + 2] <= 'f')) ||
            ((c[i + 2] >= 'A') && (c[i + 2] <= 'F'))) {
          buf[i] = c[i + 2];
        } else {
          break;
        }
      }
      buf[i] = '\0';
      int16_t imm = strtoul(buf, NULL, 16);
      struct token *token = token_create(line_number, TOK_IMMEDIATE, c, i + 2);
      token->imm = imm;
      list_append(*tokens, token);
      c += 2 + i;
      continue;
    } else if ((c[0] >= '0') && (c[0] <= '9')) {
      char buf[16];
      unsigned int i;
      for (i = 0; i < 15; i++) {
        if ((c[i] >= '0') && (c[i] <= '9')) {
          buf[i] = c[i];
        } else {
          break;
        }
      }
      buf[i] = '\0';
      struct token *token = token_create(line_number, TOK_IMMEDIATE, c, i);
      token->imm = strtoul(buf, NULL, 10);
      list_append(*tokens, token);
      c += i;
      continue;
    } else if (((c[0] >= 'a') && (c[0] <= 'z')) ||
               ((c[0] >= 'A') && (c[0] <= 'Z'))) {
      unsigned int i;
      for (i = 0; i < 255; i++) {
        if (((c[i] >= 'a') && (c[i] <= 'z')) ||
            ((c[i] >= 'A') && (c[i] <= 'Z')) ||
            ((c[i] >= '0') && (c[i] <= '9')) || (c[i] == '_')) {
          continue;
        } else {
          break;
        }
      };
      struct token *token = token_create(line_number, TOK_IDENTIFIER, c, i);
      list_append(*tokens, token);
      c += i;
      continue;
    } else if (c[0] == '"') {
      char string_buf[4096];
      unsigned int j = 0;
      unsigned int i;
      for (i = 1; i < 4096; i++) {
        if (c[i] == '\0') {
          return ERROR_UNTERMINATED_STRING;
        }
        if (c[i] == '\\') {
          if (c[i + 1] == '\0') {
            return ERROR_UNTERMINATED_STRING;
          }
          if (c[i + 1] == 'r') {
            string_buf[j++] = '\r';
          } else if (c[i + 1] == 'n') {
            string_buf[j++] = '\n';
          } else if (c[i + 1] == 't') {
            string_buf[j++] = 't';
          } else if (c[i + 1] == '"') {
            string_buf[j++] = '"';
          } else {
            return ERROR_INVALID_ESCAPE_CHAR;
          }
          i++;
          continue;
        }
        if (c[i] == '"')
          break;
        string_buf[j++] = c[i];
      }
      if (i == 4096) {
        return ERROR_STRING_TOO_LONG;
      }
      string_buf[j] = '\0';
      struct token *token =
          token_create(line_number, TOK_STRING, string_buf, j);
      list_append(*tokens, token);
      c += i + 1;
      continue;
    } else {
      char line_buf[128];
      unsigned int i;
      for (i = 0; i < 127; i++) {
        if ((c[i] == '\0') || (c[i] == '\n')) {
          break;
        }
        line_buf[i] = c[i];
      }
      line_buf[i] = '\0';
      snprintf(error_description, ERROR_DESCRIPTION_SIZE,
               "Lexer error, line: %u (%s)", line_number, line_buf);
      list_delete(*tokens, (void (*)(void *))token_delete);
      return ERROR_LEXER;
    }
  }
  struct token *token = token_create(0, TOK_NEWLINE, "", 0);
  list_append(*tokens, token);
  return SUCCESS;
}