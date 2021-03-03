#include "../rust.h"
#include "ast.h"
#include <stdlib.h>
#include <string.h>
const static struct object ast_object = {(object_delete_f)ast_delete,
                                         (object_copy_f)ast_copy,
                                         (object_cmp_f)object_not_comparable};
const struct object *object_type_ast = &ast_object;
struct ast *ast_create(enum ast_type type) {
  struct ast *ast = (struct ast *)unwrap_null(malloc(sizeof(struct ast)));
  ast->object = &ast_object;
  ast->type = type;
  return ast;
}
void ast_delete(struct ast *ast) {
  switch (ast->type) {
  case AST_ADD:
  case AST_REMOVE:
  case AST_DOT_GRAPH:
  case AST_EXIT:
  case AST_TRANSITIVE_CLOSURE:
    free(ast);
    break;
  case AST_STATEMENT:
    ast_delete(ast->statement.ast);
    free(ast);
    break;
  case AST_WHERE_IN:
    ast_delete(ast->where_in.nodes);
    ast_delete(ast->where_in.filter);
    free(ast);
    break;
  case AST_WHERE_NOT_IN:
    ast_delete(ast->where_not_in.nodes);
    ast_delete(ast->where_not_in.filter);
    free(ast);
    break;
  case AST_GET:
    vertex_identifier_delete(ast->get.vi);
    free(ast);
    break;
  case AST_FOR:
    vertex_identifier_delete(ast->for_.vi);
    ast_delete(ast->for_.get);
    ast_delete(ast->for_.body);
    free(ast);
    break;
  case AST_NODES:
    list_delete(ast->nodes.nodes);
    free(ast);
    break;
  case AST_SET_SUCCESSORS:
    ast_delete(ast->set_successors.get);
    ast_delete(ast->set_successors.successors);
    free(ast);
    break;
  case AST_SET_PREDECESSORS:
    ast_delete(ast->set_predecessors.get);
    ast_delete(ast->set_predecessors.predecessors);
    free(ast);
    break;
  case AST_ADD_SUCCESSORS:
    ast_delete(ast->add_successors.nodes);
    ast_delete(ast->add_successors.successors);
    free(ast);
    break;
  case AST_ADD_PREDECESSORS:
    ast_delete(ast->add_predecessors.nodes);
    ast_delete(ast->add_predecessors.predecessors);
    free(ast);
    break;
  case AST_GET_PREDECESSORS:
    ast_delete(ast->get_predecessors.get);
    free(ast);
    break;
  case AST_GET_SUCCESSORS:
    ast_delete(ast->get_successors.get);
    free(ast);
    break;
  case AST_UPDATE_NODE_ID:
    ast_delete(ast->update_node_id.nodes);
    free(ast);
  case AST_TOKEN:
    token_delete(ast->token);
    free(ast);
    break;
  case AST_SET_PROPERTY:
    ast_delete(ast->set_property.nodes);
    free((void *)ast->set_property.key);
    free((void *)ast->set_property.value);
    free(ast);
    break;
  case AST_HAS_PROPERTY:
    free((void *)ast->has_property.key);
    free(ast);
    break;
  case AST_PROPERTY_IS:
    free((void *)ast->property_is.key);
    free((void *)ast->property_is.value);
    free(ast);
    break;
  case AST_REMOVE_PROPERTY:
    ast_delete(ast->remove_property.nodes);
    free((void *)ast->remove_property.key);
    free(ast);
    break;
  case AST_SORT_BY:
    ast_delete(ast->sort_by.nodes);
    free((void *)ast->sort_by.key);
    free(ast);
    break;
  case AST_END:
    break;
  }
}
struct ast *ast_copy(const struct ast *ast) {
  struct ast *new_ast = ast_create(ast->type);
  switch (ast->type) {
  case AST_TRANSITIVE_CLOSURE:
  case AST_EXIT:
  case AST_DOT_GRAPH:
    break;
  case AST_STATEMENT:
    new_ast->statement.ast = object_copy(ast->statement.ast);
    break;
  case AST_ADD:
    new_ast->add.node_id = ast->add.node_id;
    break;
  case AST_REMOVE:
    new_ast->remove.node_id = ast->remove.node_id;
    break;
  case AST_NODES:
    new_ast->nodes.nodes = list_copy(ast->nodes.nodes);
    break;
  case AST_GET:
    new_ast->get.vi = object_copy(ast->get.vi);
    break;
  case AST_WHERE_IN:
    new_ast->where_in.nodes = object_copy(ast->where_in.nodes);
    new_ast->where_in.filter = object_copy(ast->where_in.filter);
    break;
  case AST_WHERE_NOT_IN:
    new_ast->where_in.nodes = object_copy(ast->where_in.nodes);
    new_ast->where_in.filter = object_copy(ast->where_in.filter);
    break;
  case AST_FOR:
    new_ast->for_.vi = object_copy(ast->for_.vi);
    new_ast->for_.get = object_copy(ast->for_.get);
    new_ast->for_.body = object_copy(ast->for_.body);
    break;
  case AST_SET_SUCCESSORS:
    new_ast->set_successors.get = object_copy(ast->set_successors.get);
    new_ast->set_successors.successors =
        object_copy(ast->set_successors.successors);
    break;
  case AST_SET_PREDECESSORS:
    new_ast->set_predecessors.get = object_copy(ast->set_predecessors.get);
    new_ast->set_predecessors.predecessors =
        object_copy(ast->set_predecessors.predecessors);
    break;
  case AST_GET_PREDECESSORS:
    new_ast->get_predecessors.get = object_copy(ast->get_predecessors.get);
    break;
  case AST_GET_SUCCESSORS:
    new_ast->get_successors.get = object_copy(ast->get_successors.get);
    break;
  case AST_ADD_SUCCESSORS:
    new_ast->add_successors.nodes = object_copy(ast->add_successors.nodes);
    new_ast->add_successors.successors =
        object_copy(ast->add_successors.successors);
    break;
  case AST_ADD_PREDECESSORS:
    new_ast->add_predecessors.nodes = object_copy(ast->add_predecessors.nodes);
    new_ast->add_predecessors.predecessors =
        object_copy(ast->add_predecessors.predecessors);
    break;
  case AST_UPDATE_NODE_ID:
    new_ast->update_node_id.nodes = object_copy(ast->update_node_id.nodes);
    new_ast->update_node_id.node_id = ast->update_node_id.node_id;
    break;
  case AST_TOKEN:
    new_ast->token = object_copy(ast->token);
    break;
  case AST_SET_PROPERTY:
    new_ast->set_property.nodes = object_copy(ast->set_property.nodes);
    new_ast->set_property.key = malloc(ast->set_property.key_length);
    memcpy(new_ast->set_property.key, ast->set_property.key,
           ast->set_property.key_length);
    new_ast->set_property.key_length = ast->set_property.key_length;
    new_ast->set_property.value = malloc(ast->set_property.value_length);
    memcpy(new_ast->set_property.value, ast->set_property.value,
           ast->set_property.value_length);
    new_ast->set_property.value_length = ast->set_property.value_length;
    break;
  case AST_HAS_PROPERTY:
    new_ast->has_property.key = malloc(ast->has_property.key_length);
    memcpy(new_ast->has_property.key, ast->has_property.key,
           ast->has_property.key_length);
    new_ast->has_property.key_length = ast->has_property.key_length;
    break;
  case AST_PROPERTY_IS:
    new_ast->property_is.key = malloc(ast->property_is.key_length);
    memcpy(new_ast->property_is.key, ast->property_is.key,
           ast->property_is.key_length);
    new_ast->property_is.key_length = ast->property_is.key_length;
    new_ast->property_is.value = malloc(ast->property_is.value_length);
    memcpy(new_ast->property_is.value, ast->property_is.value,
           ast->property_is.value_length);
    new_ast->property_is.value_length = ast->property_is.value_length;
    break;
  case AST_REMOVE_PROPERTY:
    new_ast->remove_property.nodes = object_copy(ast->remove_property.nodes);
    new_ast->remove_property.key = malloc(ast->remove_property.key_length);
    memcpy(new_ast->remove_property.key, ast->remove_property.key,
           ast->remove_property.key_length);
    new_ast->remove_property.key_length = ast->remove_property.key_length;
    break;
  case AST_SORT_BY:
    new_ast->sort_by.nodes = object_copy(ast->sort_by.nodes);
    new_ast->sort_by.key = malloc(ast->sort_by.key_length);
    memcpy(new_ast->sort_by.key, ast->sort_by.key, ast->sort_by.key_length);
    new_ast->sort_by.key_length = ast->sort_by.key_length;
    break;
  case AST_END:
    break;
  }
  return new_ast;
}
const char *ast_name(const struct ast *ast) {
  switch (ast->type) {
  case AST_STATEMENT:
    return "ast_statement";
  case AST_ADD:
    return "ast_add";
  case AST_REMOVE:
    return "ast_remove";
  case AST_NODES:
    return "ast_nodes";
  case AST_GET:
    return "ast_get";
  case AST_FOR:
    return "ast_for";
  case AST_WHERE_IN:
    return "ast_where_in";
  case AST_WHERE_NOT_IN:
    return "ast_where_not_in";
  case AST_SET_SUCCESSORS:
    return "ast_set_successors";
  case AST_SET_PREDECESSORS:
    return "ast_set_predecessors";
  case AST_GET_PREDECESSORS:
    return "ast_get_predecessors";
  case AST_GET_SUCCESSORS:
    return "ast_get_successors";
  case AST_ADD_PREDECESSORS:
    return "ast_add_predecessors";
  case AST_ADD_SUCCESSORS:
    return "ast_add_successors";
  case AST_TOKEN:
    return "ast_token";
  case AST_TRANSITIVE_CLOSURE:
    return "ast_transitive_closure";
  case AST_DOT_GRAPH:
    return "ast_dot_graph";
  case AST_END:
    return "ast_end";
  case AST_SORT_BY:
    return "ast_sort_by";
  default:
    panic("unknown ast type");
  }
}
struct ast *ast_create_statement(struct ast *ast) {
  struct ast *statement = ast_create(AST_STATEMENT);
  statement->statement.ast = ast;
  return statement;
}
struct ast *ast_create_add(uint64_t node_id) {
  struct ast *ast = ast_create(AST_ADD);
  ast->add.node_id = node_id;
  return ast;
}
struct ast *ast_create_remove(uint64_t node_id) {
  struct ast *ast = ast_create(AST_REMOVE);
  ast->remove.node_id = node_id;
  return ast;
}
struct ast *ast_create_get(struct vertex_identifier *vi) {
  struct ast *ast = ast_create(AST_GET);
  ast->get.vi = vi;
  return ast;
}
struct ast *ast_create_for(struct vertex_identifier *vi, struct ast *get,
                           struct ast *body) {
  struct ast *ast = ast_create(AST_FOR);
  ast->for_.vi = vi;
  ast->for_.get = get;
  ast->for_.body = body;
  return ast;
}
struct ast *ast_create_where_in(struct ast *nodes, struct ast *filter) {
  struct ast *ast = ast_create(AST_WHERE_IN);
  ast->where_in.nodes = nodes;
  ast->where_in.filter = filter;
  return ast;
}
struct ast *ast_create_where_not_in(struct ast *nodes, struct ast *filter) {
  struct ast *ast = ast_create(AST_WHERE_NOT_IN);
  ast->where_not_in.nodes = nodes;
  ast->where_not_in.filter = filter;
  return ast;
}
struct ast *ast_create_nodes(struct ast *node) {
  struct ast *ast = ast_create(AST_NODES);
  ast->nodes.nodes = list_create(object_type_ast);
  list_append(ast->nodes.nodes, node);
  return ast;
}
void ast_nodes_append(struct ast *ast, struct ast *node) {
  list_append(ast->nodes.nodes, node);
}
struct ast *ast_create_set_successors(struct ast *get, struct ast *successors) {
  struct ast *ast = ast_create(AST_SET_SUCCESSORS);
  ast->set_successors.get = get;
  ast->set_successors.successors = successors;
  return ast;
}
struct ast *ast_create_set_predecessors(struct ast *get,
                                        struct ast *predecessors) {
  struct ast *ast = ast_create(AST_SET_PREDECESSORS);
  ast->set_predecessors.get = get;
  ast->set_predecessors.predecessors = predecessors;
  return ast;
}
struct ast *ast_create_update_node_id(struct ast *nodes, uint64_t node_id) {
  struct ast *ast = ast_create(AST_UPDATE_NODE_ID);
  ast->update_node_id.nodes = nodes;
  ast->update_node_id.node_id = node_id;
  return ast;
}
struct ast *ast_create_get_predecessors(struct ast *get) {
  struct ast *ast = ast_create(AST_GET_PREDECESSORS);
  ast->get_predecessors.get = get;
  return ast;
}
struct ast *ast_create_get_successors(struct ast *get) {
  struct ast *ast = ast_create(AST_GET_SUCCESSORS);
  ast->get_successors.get = get;
  return ast;
}
struct ast *ast_create_add_successors(struct ast *nodes,
                                      struct ast *successors) {
  struct ast *ast = ast_create(AST_ADD_SUCCESSORS);
  ast->add_successors.nodes = nodes;
  ast->add_successors.successors = successors;
  return ast;
}
struct ast *ast_create_add_predecessors(struct ast *nodes,
                                        struct ast *predecessors) {
  struct ast *ast = ast_create(AST_ADD_PREDECESSORS);
  ast->add_predecessors.nodes = nodes;
  ast->add_predecessors.predecessors = predecessors;
  return ast;
}
struct ast *ast_create_transitive_closure() {
  return ast_create(AST_TRANSITIVE_CLOSURE);
}
struct ast *ast_create_dot_graph() {
  return ast_create(AST_DOT_GRAPH);
}
struct ast *ast_create_exit() {
  return ast_create(AST_EXIT);
}
struct ast *ast_create_token(struct token *token) {
  struct ast *ast = ast_create(AST_TOKEN);
  ast->token = token;
  return ast;
}
struct ast *ast_create_set_property(struct ast *nodes, const uint8_t *key,
                                    uint32_t key_length, const uint8_t *value,
                                    uint32_t value_length) {
  struct ast *ast = ast_create(AST_SET_PROPERTY);
  ast->set_property.nodes = nodes;
  ast->set_property.key = malloc(key_length);
  memcpy(ast->set_property.key, key, key_length);
  ast->set_property.key_length = key_length;
  ast->set_property.value = malloc(value_length);
  memcpy(ast->set_property.value, value, value_length);
  ast->set_property.value_length = value_length;
  return ast;
}
struct ast *ast_create_has_property(const uint8_t *key, uint32_t key_length) {
  struct ast *ast = ast_create(AST_HAS_PROPERTY);
  ast->has_property.key = malloc(key_length);
  memcpy(ast->has_property.key, key, key_length);
  ast->has_property.key_length = key_length;
  return ast;
}
struct ast *ast_create_property_is(const uint8_t *key, uint32_t key_length,
                                   const uint8_t *value,
                                   uint32_t value_length) {
  struct ast *ast = ast_create(AST_PROPERTY_IS);
  ast->property_is.key = malloc(key_length);
  memcpy(ast->property_is.key, key, key_length);
  ast->property_is.key_length = key_length;
  ast->property_is.value = malloc(value_length);
  memcpy(ast->property_is.value, value, value_length);
  ast->property_is.value_length = value_length;
  return ast;
}
struct ast *ast_create_remove_property(struct ast *nodes, const uint8_t *key,
                                       uint32_t key_length) {
  struct ast *ast = ast_create(AST_REMOVE_PROPERTY);
  ast->remove_property.nodes = nodes;
  ast->remove_property.key = malloc(key_length);
  memcpy(ast->remove_property.key, key, key_length);
  ast->remove_property.key_length = key_length;
  return ast;
}
struct ast *ast_create_sort_by(struct ast *nodes, const uint8_t *key,
                               uint32_t key_length) {
  struct ast *ast = ast_create(AST_SORT_BY);
  ast->sort_by.nodes = nodes;
  ast->sort_by.key = malloc(key_length);
  memcpy(ast->sort_by.key, key, key_length);
  ast->sort_by.key_length = key_length;
  return ast;
}