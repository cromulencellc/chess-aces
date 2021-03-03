#include "parser.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rust.h"
enum parser_rule_item_type { TOKEN, AST };
struct parser_rule_item {
  enum parser_rule_item_type type;
  unsigned int token;
};
enum parser_rule_type {
  PR_ADD,
  PR_REMOVE,
  PR_GET_NUMBER,
  PR_GET_LABEL,
  PR_FOR,
  PR_WHERE_IN,
  PR_WHERE_NOT_IN,
  PR_NODES_GET,
  PR_NODES_GET_SUCCESSORS,
  PR_NODES_GET_PREDECESSORS,
  PR_NODES_FOR,
  PR_NODES_WHERE_IN,
  PR_NODES_NODES,
  PR_NODES_STATEMENT,
  PR_UPDATE_NODE_ID,
  PR_GET_SUCCESSORS,
  PR_GET_PREDECESSORS,
  PR_SET_SUCCESSORS,
  PR_SET_PREDECESSORS,
  PR_SET_PROPERTY,
  PR_ADD_SUCCESSORS,
  PR_ADD_PREDECESSORS,
  PR_HAS_PROPERTY,
  PR_PROPERTY_IS,
  PR_REMOVE_PROPERTY,
  PR_TRANSITIVE_CLOSURE,
  PR_EXIT,
  PR_DOT_GRAPH,
  PR_SORT_BY,
};
const char *parser_rule_name(enum parser_rule_type parser_rule) {
  switch (parser_rule) {
  case PR_PROPERTY_IS:
    return "pr_property_is";
  case PR_HAS_PROPERTY:
    return "pr_has_property";
  case PR_SET_PROPERTY:
    return "pr_set_property";
  case PR_REMOVE_PROPERTY:
    return "pr_remove_property";
  case PR_ADD:
    return "pr_add";
  case PR_REMOVE:
    return "pr_remove";
  case PR_GET_NUMBER:
    return "pr_get_number";
  case PR_GET_LABEL:
    return "pr_get_label";
  case PR_FOR:
    return "pr_for";
  case PR_WHERE_IN:
    return "pr_where_in";
  case PR_WHERE_NOT_IN:
    return "pr_where_not_in";
  case PR_NODES_GET:
    return "pr_nodes_get";
  case PR_NODES_GET_SUCCESSORS:
    return "pr_nodes_get_successors";
  case PR_NODES_GET_PREDECESSORS:
    return "pr_nodes_get_predecessors";
  case PR_NODES_WHERE_IN:
    return "pr_nodes_where_in";
  case PR_NODES_FOR:
    return "pr_nodes_for";
  case PR_NODES_NODES:
    return "pr_nodes_nodes";
  case PR_NODES_STATEMENT:
    return "pr_nodes_statement";
  case PR_UPDATE_NODE_ID:
    return "pr_update_node_id";
  case PR_GET_SUCCESSORS:
    return "pr_get_successors";
  case PR_GET_PREDECESSORS:
    return "pr_get_predecessors";
  case PR_SET_SUCCESSORS:
    return "pr_set_successors";
  case PR_SET_PREDECESSORS:
    return "pr_set_predecessors";
  case PR_ADD_SUCCESSORS:
    return "pr_add_successors";
  case PR_ADD_PREDECESSORS:
    return "'pr_add_predecessors";
  case PR_TRANSITIVE_CLOSURE:
    return "pr_transitive_closure";
  case PR_DOT_GRAPH:
    return "pr_dot_graph";
  case PR_EXIT:
    return "pr_exit";
  case PR_SORT_BY:
    return "pr_sort_by";
  default:
    panic("unknown parser rule type");
  }
}
struct parser_rule {
  enum parser_rule_type rule;
  struct parser_rule_item items[16];
  struct parser_rule_item lookaheads[4];
};
unsigned int parser_rule_length(const struct parser_rule *parser_rule) {
  unsigned int i;
  for (i = 0; ((parser_rule->items[i].type != AST) ||
               ((parser_rule->items[i].type == AST) &&
                (parser_rule->items[i].token != AST_END)));
       i++) {
  }
  return i;
}
struct parser_rule parser_rules[] = {
    {.rule = PR_ADD,
     .items = {{.type = TOKEN, .token = TOK_ADD},
               {.type = TOKEN, .token = TOK_PAREN_OPEN},
               {.type = TOKEN, .token = TOK_NUMBER},
               {.type = TOKEN, .token = TOK_PAREN_CLOSE},
               {.type = TOKEN, .token = TOK_SEMICOLON},
               {.type = AST, .token = AST_END}},
     {{.type = AST, .token = AST_END}}},
    {PR_REMOVE,
     {{TOKEN, TOK_REMOVE},
      {TOKEN, TOK_PAREN_OPEN},
      {TOKEN, TOK_NUMBER},
      {TOKEN, TOK_PAREN_CLOSE},
      {TOKEN, TOK_SEMICOLON},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_GET_NUMBER,
     {{TOKEN, TOK_GET},
      {TOKEN, TOK_PAREN_OPEN},
      {TOKEN, TOK_NUMBER},
      {TOKEN, TOK_PAREN_CLOSE},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_GET_LABEL,
     {{TOKEN, TOK_GET},
      {TOKEN, TOK_PAREN_OPEN},
      {TOKEN, TOK_IDENTIFIER},
      {TOKEN, TOK_PAREN_CLOSE},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_FOR,
     {{TOKEN, TOK_FOR},
      {TOKEN, TOK_IDENTIFIER},
      {TOKEN, TOK_IN},
      {AST, AST_NODES},
      {AST, AST_NODES},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_WHERE_IN,
     {{TOKEN, TOK_WHERE},
      {AST, AST_NODES},
      {TOKEN, TOK_IN},
      {AST, AST_NODES},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_WHERE_NOT_IN,
     {{TOKEN, TOK_WHERE},
      {AST, AST_NODES},
      {TOKEN, TOK_NOT},
      {TOKEN, TOK_IN},
      {AST, AST_NODES},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_NODES_GET, {{AST, AST_GET}, {AST, AST_END}}, {{AST, AST_END}}},
    {PR_NODES_GET_SUCCESSORS,
     {{AST, AST_GET_SUCCESSORS}, {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_NODES_GET_PREDECESSORS,
     {{AST, AST_GET_PREDECESSORS}, {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_NODES_WHERE_IN,
     {{AST, AST_WHERE_IN}, {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_NODES_FOR, {{AST, AST_FOR}, {AST, AST_END}}, {{AST, AST_END}}},
    {PR_NODES_NODES,
     {{TOKEN, TOK_SQUARE_BRACKET_OPEN},
      {AST, AST_NODES},
      {TOKEN, TOK_COMMA},
      {AST, AST_NODES},
      {TOKEN, TOK_SQUARE_BRACKET_CLOSE},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_NODES_STATEMENT,
     {{AST, AST_NODES}, {TOKEN, TOK_SEMICOLON}, {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_UPDATE_NODE_ID,
     {
         {TOKEN, TOK_UPDATE_NODE_ID},
         {TOKEN, TOK_PAREN_OPEN},
         {AST, AST_NODES},
         {TOKEN, TOK_COMMA},
         {TOKEN, TOK_NUMBER},
         {TOKEN, TOK_PAREN_CLOSE},
         {TOKEN, TOK_SEMICOLON},
     }},
    {PR_GET_SUCCESSORS,
     {{TOKEN, TOK_GET_SUCCESSORS},
      {TOKEN, TOK_PAREN_OPEN},
      {AST, AST_NODES},
      {TOKEN, TOK_PAREN_CLOSE},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_GET_PREDECESSORS,
     {{TOKEN, TOK_GET_PREDECESSORS},
      {TOKEN, TOK_PAREN_OPEN},
      {AST, AST_NODES},
      {TOKEN, TOK_PAREN_CLOSE},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_SET_SUCCESSORS,
     {{TOKEN, TOK_SET_SUCCESSORS},
      {TOKEN, TOK_PAREN_OPEN},
      {AST, AST_NODES},
      {TOKEN, TOK_COMMA},
      {AST, AST_NODES},
      {TOKEN, TOK_PAREN_CLOSE},
      {TOKEN, TOK_SEMICOLON},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_SET_PREDECESSORS,
     {{TOKEN, TOK_SET_PREDECESSORS},
      {TOKEN, TOK_PAREN_OPEN},
      {AST, AST_NODES},
      {TOKEN, TOK_COMMA},
      {AST, AST_NODES},
      {TOKEN, TOK_PAREN_CLOSE},
      {TOKEN, TOK_SEMICOLON},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_ADD_SUCCESSORS,
     {{TOKEN, TOK_ADD_SUCCESSORS},
      {TOKEN, TOK_PAREN_OPEN},
      {AST, AST_NODES},
      {TOKEN, TOK_COMMA},
      {AST, AST_NODES},
      {TOKEN, TOK_PAREN_CLOSE},
      {TOKEN, TOK_SEMICOLON},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_ADD_PREDECESSORS,
     {{TOKEN, TOK_ADD_PREDECESSORS},
      {TOKEN, TOK_PAREN_OPEN},
      {AST, AST_NODES},
      {TOKEN, TOK_COMMA},
      {AST, AST_NODES},
      {TOKEN, TOK_PAREN_CLOSE},
      {TOKEN, TOK_SEMICOLON},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_SET_PROPERTY,
     {{TOKEN, TOK_SET_PROPERTY},
      {TOKEN, TOK_PAREN_OPEN},
      {AST, AST_NODES},
      {TOKEN, TOK_COMMA},
      {TOKEN, TOK_STRING},
      {TOKEN, TOK_COMMA},
      {TOKEN, TOK_STRING},
      {TOKEN, TOK_PAREN_CLOSE},
      {TOKEN, TOK_SEMICOLON},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_PROPERTY_IS,
     {{TOKEN, TOK_PROPERTY_IS},
      {TOKEN, TOK_PAREN_OPEN},
      {TOKEN, TOK_STRING},
      {TOKEN, TOK_COMMA},
      {TOKEN, TOK_STRING},
      {TOKEN, TOK_PAREN_CLOSE},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_HAS_PROPERTY,
     {{TOKEN, TOK_HAS_PROPERTY},
      {TOKEN, TOK_PAREN_OPEN},
      {TOKEN, TOK_STRING},
      {TOKEN, TOK_PAREN_CLOSE},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_REMOVE_PROPERTY,
     {{TOKEN, TOK_REMOVE_PROPERTY},
      {TOKEN, TOK_PAREN_OPEN},
      {AST, AST_NODES},
      {TOKEN, TOK_COMMA},
      {TOKEN, TOK_STRING},
      {TOKEN, TOK_PAREN_CLOSE},
      {TOKEN, TOK_SEMICOLON},
      {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_DOT_GRAPH,
     {{TOKEN, TOK_DOT_GRAPH}, {TOKEN, TOK_SEMICOLON}, {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_EXIT,
     {{TOKEN, TOK_EXIT}, {TOKEN, TOK_SEMICOLON}, {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_TRANSITIVE_CLOSURE,
     {{TOKEN, TOK_TRANSITIVE_CLOSURE}, {TOKEN, TOK_SEMICOLON}, {AST, AST_END}},
     {{AST, AST_END}}},
    {PR_SORT_BY,
     {{TOKEN, TOK_SORT_BY},
      {TOKEN, TOK_PAREN_OPEN},
      {AST, AST_NODES},
      {TOKEN, TOK_COMMA},
      {TOKEN, TOK_STRING},
      {TOKEN, TOK_PAREN_CLOSE},
      {AST, AST_END}},
     {{AST, AST_END}}}};
int parser_match(const struct parser *parser,
                 enum parser_rule_type *rule_type) {
  unsigned int i;
  for (i = 0; i < sizeof(parser_rules) / sizeof(struct parser_rule); i++) {
    const struct parser_rule *parser_rule = &parser_rules[i];
    unsigned int rule_length = parser_rule_length(parser_rule);
    unsigned int j;
    for (j = 0; j < rule_length; j++) {
      const struct parser_rule_item *item = &parser_rule->items[j];
      const struct ast *ast =
          stack_peek_ref(parser->stack, rule_length - j - 1);
      if (ast == NULL) {
        break;
      }
      if (item->type == TOKEN) {
        if (ast->type != AST_TOKEN) {
          break;
        }
        if (ast->token->type != item->token) {
          break;
        }
      } else {
        if (ast->type != item->token) {
          break;
        }
      }
    }
    if (j == rule_length) {
      *rule_type = parser_rule->rule;
      return 0;
    }
  }
  return -1;
}
int parser_reduce(struct parser *parser) {
  enum parser_rule_type parser_rule_type;
  while (parser_match(parser, &parser_rule_type) == 0) {
    switch (parser_rule_type) {
    case PR_TRANSITIVE_CLOSURE: {
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      stack_push(parser->stack,
                 ast_create_statement(ast_create_transitive_closure()));
      break;
    }
    case PR_DOT_GRAPH: {
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      stack_push(parser->stack, ast_create_statement(ast_create_dot_graph()));
      break;
    }
    case PR_EXIT: {
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      stack_push(parser->stack, ast_create_statement(ast_create_exit()));
      break;
    }
    case PR_ADD:
    case PR_REMOVE: {
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      struct ast *tok_num = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      struct ast *ast;
      if (parser_rule_type == PR_ADD) {
        ast = ast_create_statement(ast_create_add(tok_num->token->num));
      } else {
        ast = ast_create_statement(ast_create_remove(tok_num->token->num));
      }
      object_delete(tok_num);
      stack_push(parser->stack, ast);
      break;
    }
    case PR_GET_NUMBER:
    case PR_GET_LABEL: {
      object_delete(stack_pop(parser->stack));
      struct ast *number_label = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      struct ast *ast;
      if (parser_rule_type == PR_GET_NUMBER) {
        ast = ast_create_get(
            vertex_identifier_create_node_id(number_label->token->num));
      } else {
        ast = ast_create_get(
            vertex_identifier_create_label(number_label->token->text));
      }
      object_delete(number_label);
      stack_push(parser->stack, ast);
      break;
    }
    case PR_WHERE_IN: {
      struct ast *filter = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *nodes = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *ast = ast_create_where_in(nodes, filter);
      stack_push(parser->stack, ast);
      break;
    }
    case PR_WHERE_NOT_IN: {
      struct ast *filter = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      struct ast *nodes = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *ast =
          ast_create_nodes(ast_create_where_not_in(nodes, filter));
      stack_push(parser->stack, ast);
      break;
    }
    case PR_FOR: {
      struct ast *body = stack_pop(parser->stack);
      struct ast *get = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *identifier = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *ast = ast_create_for(
          vertex_identifier_create_label(identifier->token->text), get, body);
      object_delete(identifier);
      stack_push(parser->stack, ast);
      break;
    }
    case PR_NODES_GET: {
      struct ast *ast_get = stack_pop(parser->stack);
      stack_push(parser->stack, ast_create_nodes(ast_get));
      break;
    }
    case PR_NODES_GET_SUCCESSORS: {
      struct ast *ast_get_successors = stack_pop(parser->stack);
      stack_push(parser->stack, ast_create_nodes(ast_get_successors));
      break;
    }
    case PR_NODES_GET_PREDECESSORS: {
      struct ast *ast_get_predecessors = stack_pop(parser->stack);
      stack_push(parser->stack, ast_create_nodes(ast_get_predecessors));
      break;
    }
    case PR_NODES_WHERE_IN: {
      struct ast *ast = stack_pop(parser->stack);
      assert(ast->type == AST_WHERE_IN);
      stack_push(parser->stack, ast_create_nodes(ast));
      break;
    }
    case PR_NODES_FOR: {
      struct ast *ast = stack_pop(parser->stack);
      stack_push(parser->stack, ast_create_nodes(ast));
      break;
    }
    case PR_NODES_NODES: {
      object_delete(stack_pop(parser->stack));
      struct ast *nodes1 = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *nodes2 = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      list_join(nodes1->nodes.nodes, list_copy(nodes2->nodes.nodes));
      object_delete(nodes2);
      stack_push(parser->stack, nodes1);
      break;
    }
    case PR_NODES_STATEMENT: {
      object_delete(stack_pop(parser->stack));
      struct ast *nodes = stack_pop(parser->stack);
      stack_push(parser->stack, ast_create_statement(nodes));
      break;
    }
    case PR_UPDATE_NODE_ID: {
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      struct ast *node_id = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *nodes = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      stack_push(parser->stack, ast_create_statement(ast_create_update_node_id(
                                    nodes, node_id->token->num)));
      object_delete(node_id);
      break;
    }
    case PR_GET_SUCCESSORS: {
      object_delete(stack_pop(parser->stack));
      struct ast *nodes = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      stack_push(parser->stack, ast_create_get_successors(nodes));
      break;
    }
    case PR_GET_PREDECESSORS: {
      object_delete(stack_pop(parser->stack));
      struct ast *nodes = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      stack_push(parser->stack, ast_create_get_predecessors(nodes));
      break;
    }
    case PR_SET_PREDECESSORS: {
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      struct ast *predecessors = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *nodes = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      stack_push(parser->stack,
                 ast_create_statement(
                     ast_create_set_predecessors(nodes, predecessors)));
      break;
    }
    case PR_SET_SUCCESSORS: {
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      struct ast *successors = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *nodes = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      stack_push(parser->stack, ast_create_statement(ast_create_set_successors(
                                    nodes, successors)));
      break;
    }
    case PR_ADD_SUCCESSORS: {
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      struct ast *successors = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *nodes = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      stack_push(parser->stack, ast_create_statement(ast_create_add_successors(
                                    nodes, successors)));
      break;
    }
    case PR_ADD_PREDECESSORS: {
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      struct ast *successors = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *nodes = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      stack_push(
          parser->stack,
          ast_create_statement(ast_create_add_predecessors(nodes, successors)));
      break;
    }
    case PR_SET_PROPERTY: {
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      struct ast *value = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *key = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *nodes = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      stack_push(
          parser->stack,
          ast_create_statement(ast_create_set_property(
              nodes, (const uint8_t *)key->token->text, key->token->text_len,
              (const uint8_t *)value->token->text, value->token->text_len)));
      object_delete(key);
      object_delete(value);
      break;
    }
    case PR_HAS_PROPERTY: {
      object_delete(stack_pop(parser->stack));
      struct ast *key = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      stack_push(parser->stack,
                 ast_create_nodes(ast_create_has_property(
                     (const uint8_t *)key->token->text, key->token->text_len)));
      object_delete(key);
      break;
    }
    case PR_PROPERTY_IS: {
      object_delete(stack_pop(parser->stack));
      struct ast *value = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *key = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      stack_push(
          parser->stack,
          ast_create_nodes(ast_create_property_is(
              (const uint8_t *)key->token->text, key->token->text_len,
              (const uint8_t *)value->token->text, value->token->text_len)));
      object_delete(key);
      object_delete(value);
      break;
    }
    case PR_REMOVE_PROPERTY: {
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      struct ast *key = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *nodes = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      stack_push(parser->stack, ast_create_statement(ast_create_remove_property(
                                    nodes, (const uint8_t *)key->token->text,
                                    key->token->text_len)));
      object_delete(key);
      break;
    }
    case PR_SORT_BY: {
      object_delete(stack_pop(parser->stack));
      struct ast *key = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      struct ast *nodes = stack_pop(parser->stack);
      object_delete(stack_pop(parser->stack));
      object_delete(stack_pop(parser->stack));
      stack_push(parser->stack, ast_create_nodes(ast_create_sort_by(
                                    nodes, (const uint8_t *)key->token->text,
                                    key->token->text_len)));
      object_delete(key);
      break;
    }
    }
  }
  return 0;
}
struct list *parse(struct list *tokens) {
  struct parser *parser = parser_create();
  struct list_it *it;
  for (it = tokens->head; it != NULL; it = it->next) {
    const struct token *token = (const struct token *)it->object;
    stack_push(parser->stack, ast_create_token(object_copy(token)));
    parser_reduce(parser);
  }
  struct list *statements = list_create(object_type_ast);
  while (stack_num_items(parser->stack) > 0) {
    struct ast *statement = stack_pop(parser->stack);
    if (statement->type != AST_STATEMENT) {
      DEBUG("%s", ast_name(statement));
      panic("Parser error");
    }
    list_append(statements, statement);
  }
  parser_delete(parser);
  return statements;
}