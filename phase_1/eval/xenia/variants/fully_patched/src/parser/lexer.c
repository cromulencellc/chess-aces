#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "o/token.h"
#include "rust.h"
struct token_matcher {
  enum token_type token;
  const char *text;
};
struct token_matcher token_matchers[] = {
    {TOK_SET_SUCCESSORS, "set_successors"},
    {TOK_SET_PREDECESSORS, "set_predecessors"},
    {TOK_GET_PREDECESSORS, "get_predecessors"},
    {TOK_GET_SUCCESSORS, "get_successors"},
    {TOK_SET_PROPERTY, "set_property"},
    {TOK_UPDATE_NODE_ID, "update_node_id"},
    {TOK_ADD_SUCCESSORS, "add_successors"},
    {TOK_ADD_PREDECESSORS, "add_predecessors"},
    {TOK_PROPERTY_IS, "property_is"},
    {TOK_HAS_PROPERTY, "has_property"},
    {TOK_REMOVE_PROPERTY, "remove_property"},
    {TOK_TRANSITIVE_CLOSURE, "transitive_closure"},
    {TOK_DOT_GRAPH, "dot_graph"},
    {TOK_FOR, "for"},
    {TOK_IN, "in"},
    {TOK_NOT, "not"},
    {TOK_END, "end"},
    {TOK_REMOVE, "remove"},
    {TOK_ADD, "add"},
    {TOK_GET, "get"},
    {TOK_WHERE, "where"},
    {TOK_SQUARE_BRACKET_OPEN, "["},
    {TOK_SQUARE_BRACKET_CLOSE, "]"},
    {TOK_PAREN_OPEN, "("},
    {TOK_PAREN_CLOSE, ")"},
    {TOK_COMMA, ","},
    {TOK_SEMICOLON, ";"},
    {TOK_EXIT, "exit"},
    {TOK_SORT_BY, "sort_by"}};
int strncmpi(const char *lhs, const char *rhs, unsigned int length) {
  while ((*lhs != '\0') && (*rhs != '\0') && (length > 0)) {
    char l = *lhs;
    char r = *rhs;
    if ((l >= 'A') && (l <= 'Z'))
      l -= 'a' - 'A';
    if ((r >= 'A') && (r <= 'Z'))
      r -= 'a' - 'A';
    if (l != r)
      break;
    length--;
    lhs++;
    rhs++;
  }
  if (length == 0) {
    return 0;
  } else if (*lhs < *rhs) {
    return -1;
  } else if (*lhs > *rhs) {
    return 1;
  } else {
    return 0;
  }
}
const struct token_matcher *match_token(const char *s) {
  unsigned int i;
  for (i = 0; i < sizeof(token_matchers) / sizeof(struct token_matcher); i++) {
    if (strncmpi(s, token_matchers[i].text, strlen(token_matchers[i].text)) ==
        0) {
      return &token_matchers[i];
    }
  }
  return NULL;
}
int lexer(const char *text, struct list **tokens) {
  *tokens = list_create(object_type_token);
  const char *c = text;
  while (*c != 0) {
    if (*c == '#') {
      while ((*c != '\n') && (*c != '\0')) {
        c++;
      }
      continue;
    }
    if ((*c == '\t') || (*c == ' ') || (*c == '\r') || (*c == '\n')) {
      c++;
      continue;
    }
    const struct token_matcher *token_matcher = match_token(c);
    if (token_matcher != NULL) {
      struct token *token =
          token_create(token_matcher->token, c, strlen(token_matcher->text));
      list_append(*tokens, token);
      c += strlen(token_matcher->text);
      continue;
    }
    if ((*c == '0') && (c[1] == 'x')) {
      char buf[32];
      unsigned int i;
      for (i = 0; i < 31; i++) {
        if (((c[i + 2] >= '0') && (c[i + 2] <= '9')) ||
            ((c[i + 2] >= 'a') && (c[i + 2] <= 'f')) ||
            ((c[i + 2] >= 'A') && (c[i + 2] <= 'F'))) {
          buf[i] = c[i + 2];
        } else {
          break;
        }
      }
      buf[i] = '\0';
      uint64_t num = strtoull(buf, NULL, 16);
      struct token *token = token_create(TOK_NUMBER, c, i + 2);
      token->num = num;
      list_append(*tokens, token);
      c += 2 + i;
      continue;
    } else if ((c[0] >= '0') && (c[0] <= '9')) {
      char buf[32];
      unsigned int i;
      for (i = 0; i < 31; i++) {
        if ((c[i] >= '0') && (c[i] <= '9')) {
          buf[i] = c[i];
        } else {
          break;
        }
      }
      buf[i] = '\0';
      struct token *token = token_create(TOK_NUMBER, c, i);
      token->num = strtoull(buf, NULL, 10);
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
      struct token *token = token_create(TOK_IDENTIFIER, c, i);
      list_append(*tokens, token);
      c += i;
      continue;
    } else if (c[0] == '"') {
      char string_buf[8192];
      unsigned int j = 0;
      unsigned int i;
      for (i = 1; i < 8192; i++) {
        if (c[i] == '\0') {
          return -1;
        }
        if (c[i] == '\\') {
          if (c[i + 1] == '\0') {
            return -1;
          }
          if (c[i + 1] == 'r') {
            string_buf[j++] = '\r';
          } else if (c[i + 1] == 'n') {
            string_buf[j++] = '\n';
          } else if (c[i + 1] == 't') {
            string_buf[j++] = 't';
          } else if (c[i + 1] == '"') {
            string_buf[j++] = '"';
          } else if (c[i + 1] == 'x') {
            if ((c[i + 2] & c[i + 3]) == 0) {
              return -1;
            }
            unsigned char hi = (unsigned char)c[i + 2];
            if ((hi >= '0') && (hi <= '9')) {
              hi -= '0';
            } else if ((hi >= 'a') && (hi <= 'f')) {
              hi = hi - 'a' + 10;
            } else {
              return -1;
            }
            unsigned char lo = (unsigned char)c[i + 3];
            if ((lo >= '0') && (lo <= '9')) {
              lo -= '0';
            } else if ((lo >= 'a') || (lo <= 'f')) {
              lo = lo - 'a' + 10;
            } else {
              return -1;
            }
            string_buf[j++] = (char)((hi << 4) | lo);
            i += 3;
            continue;
          } else {
            return -1;
          }
          i++;
          continue;
        }
        if (c[i] == '"')
          break;
        string_buf[j++] = c[i];
      }
      if (i == 8192) {
        return -1;
      }
      string_buf[j] = '\0';
      struct token *token = token_create(TOK_STRING, string_buf, j + 1);
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
      char error_str[256];
      snprintf(error_str, 256, "Lexer error (%s)", line_buf);
      list_delete(*tokens);
      panic(error_str);
    }
  }
  return 0;
}