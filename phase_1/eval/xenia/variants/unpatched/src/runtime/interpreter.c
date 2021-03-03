#include "interpreter.h"
#include <stdlib.h>
#include <string.h>
#include "container/list.h"
#include "graphdb/graph.h"
#include "graphdb/vertex.h"
#include "graphdb/vertex_property.h"
#include "interpreter.h"
#include "rust.h"
struct interpreter *interpreter_create() {
  struct interpreter *interpreter =
      unwrap_null(malloc(sizeof(struct interpreter)));
  interpreter->graph = graph_create();
  interpreter->scope = NULL;
  interpreter->exited = 0;
  return interpreter;
}
void interpreter_delete(struct interpreter *interpreter) {
  graph_delete(interpreter->graph);
  free(interpreter);
}
int interpreter_add(struct interpreter *interpreter, const struct ast *ast) {
  if (ast->type != AST_ADD) {
    return -1;
  }
  return graph_add_vertex(interpreter->graph, ast->add.node_id);
}
int interpreter_remove(struct interpreter *interpreter, const struct ast *ast) {
  if (ast->type != AST_REMOVE) {
    return -1;
  }
  return graph_remove_vertex(interpreter->graph, ast->remove.node_id);
}
struct list *interpreter_get(struct interpreter *interpreter,
                             const struct ast *ast) {
  if (ast->type != AST_GET) {
    return NULL;
  }
  if (ast->get.vi->type == VI_NODE_ID) {
    struct list *vertices = list_create(object_type_vertex);
    struct vertex *vertex =
        graph_get_vertex_ref(interpreter->graph, ast->get.vi->node_id);
    if (vertex == NULL) {
      panic("Tried to get a vertex that does not exist.");
    }
    list_append(vertices, vertex);
    return vertices;
  } else {
    struct list *vertex_objects =
        scope_get_variable_ref(interpreter->scope, ast->get.vi->label);
    if (vertex_objects == NULL) {
      DEBUG("did not find variable %s", ast->get.vi->label);
      panic("Did not find variable");
    }
    struct list *vertices =
        vertex_object_list_to_vertex_list(vertex_objects, interpreter->graph);
    if (vertices == NULL) {
      panic("Error converting vertex objects to vertices");
    }
    return vertices;
  }
}
struct list *interpreter_where_in(struct interpreter *interpreter,
                                  const struct ast *ast) {
  if (ast->type != AST_WHERE_IN) {
    return NULL;
  }
  struct list *nodes = interpreter_nodes(interpreter, ast->where_in.nodes);
  if (nodes == NULL) {
    return NULL;
  }
  struct list *filter = interpreter_nodes(interpreter, ast->where_in.filter);
  if (filter == NULL) {
    list_delete_shallow(nodes);
    return NULL;
  }
  struct list *result = list_create(object_type_vertex);
  struct list_it *nodes_it;
  for (nodes_it = nodes->head; nodes_it != NULL; nodes_it = nodes_it->next) {
    struct list_it *filter_it;
    struct vertex *nodes_vertex = (struct vertex *)nodes_it->object;
    for (filter_it = filter->head; filter_it != NULL;
         filter_it = filter_it->next) {
      struct vertex *filter_vertex = (struct vertex *)filter_it->object;
      if (nodes_vertex->node_id == filter_vertex->node_id) {
        list_append(result, nodes_vertex);
        break;
      }
    }
  }
  list_delete_shallow(nodes);
  list_delete_shallow(filter);
  return result;
}
struct list *interpreter_where_not_in(struct interpreter *interpreter,
                                      const struct ast *ast) {
  if (ast->type != AST_WHERE_NOT_IN) {
    return NULL;
  }
  struct list *nodes = interpreter_nodes(interpreter, ast->where_in.nodes);
  if (nodes == NULL) {
    return NULL;
  }
  struct list *filter = interpreter_nodes(interpreter, ast->where_in.filter);
  if (filter == NULL) {
    list_delete_shallow(nodes);
    return NULL;
  }
  struct list *result = list_create(object_type_vertex);
  struct list_it *nodes_it;
  for (nodes_it = nodes->head; nodes_it != NULL; nodes_it = nodes_it->next) {
    struct list_it *filter_it;
    struct vertex *nodes_vertex = (struct vertex *)nodes_it->object;
    unsigned int found = 0;
    for (filter_it = filter->head; filter_it != NULL;
         filter_it = filter_it->next) {
      struct vertex *filter_vertex = (struct vertex *)filter_it->object;
      if (nodes_vertex->node_id == filter_vertex->node_id) {
        found = 1;
        break;
      }
    }
    if (found == 0) {
      list_append(result, nodes_vertex);
    }
  }
  list_delete_shallow(nodes);
  list_delete_shallow(filter);
  return result;
}
struct list *interpreter_for(struct interpreter *interpreter,
                             const struct ast *ast) {
  if (ast->type != AST_FOR) {
    return NULL;
  }
  struct list *vertices = interpreter_eval(interpreter, ast->for_.get);
  interpreter->scope = scope_push(interpreter->scope);
  struct list *vertex_objects = vertex_list_to_vertex_object_list(vertices);
  list_delete_shallow(vertices);
  scope_set_variable(interpreter->scope, ast->for_.vi->label, vertex_objects);
  vertices = interpreter_eval(interpreter, ast->for_.body);
  interpreter->scope = scope_pop(interpreter->scope);
  return vertices;
}
struct list *interpreter_get_predecessors(struct interpreter *interpreter,
                                          struct ast *ast) {
  if (ast->type != AST_GET_PREDECESSORS) {
    return NULL;
  }
  struct list *nodes =
      unwrap_null(interpreter_nodes(interpreter, ast->get_predecessors.get));
  struct list *predecessors = list_create(object_type_vertex);
  struct list_it *it;
  for (it = nodes->head; it != NULL; it = it->next) {
    struct vertex *v = (struct vertex *)it->object;
    struct list *node_predecessors =
        unwrap_null(vertex_get_predecessors_ref(v));
    list_join(predecessors, node_predecessors);
  }
  list_delete_shallow(nodes);
  list_sort(&predecessors);
  list_dedup_shallow(predecessors);
  return predecessors;
}
struct list *interpreter_get_successors(struct interpreter *interpreter,
                                        struct ast *ast) {
  if (ast->type != AST_GET_SUCCESSORS) {
    return NULL;
  }
  struct list *nodes =
      unwrap_null(interpreter_nodes(interpreter, ast->get_successors.get));
  struct list *successors = list_create(object_type_vertex);
  struct list_it *it;
  for (it = nodes->head; it != NULL; it = it->next) {
    struct vertex *v = (struct vertex *)it->object;
    struct list *node_successors = unwrap_null(vertex_get_successors_ref(v));
    list_join(successors, node_successors);
  }
  list_delete_shallow(nodes);
  list_sort(&successors);
  list_dedup_shallow(successors);
  return successors;
}
uint8_t *interpreter_sort_by_cmp_key = NULL;
uint32_t interpreter_sort_by_cmp_key_length = 0;
int interpreter_sort_by_cmp(const struct vertex *lhs,
                            const struct vertex *rhs) {
  uint8_t *lhs_value;
  uint32_t lhs_value_size;
  uint8_t *rhs_value;
  uint32_t rhs_value_size;
  int lhs_result = vertex_get_property(
      (struct vertex *)lhs, interpreter_sort_by_cmp_key,
      interpreter_sort_by_cmp_key_length, &lhs_value, &lhs_value_size);
  int rhs_result = vertex_get_property(
      (struct vertex *)rhs, interpreter_sort_by_cmp_key,
      interpreter_sort_by_cmp_key_length, &rhs_value, &rhs_value_size);
  if ((lhs_result == 0) && (rhs_result == 0)) {
    if (lhs_value_size < rhs_value_size) {
      return -1;
    } else if (lhs_value_size > rhs_value_size) {
      return 1;
    } else {
      return memcmp(lhs_value, rhs_value, lhs_value_size);
    }
  } else if ((lhs_result == 0) && (rhs_result != 0)) {
    return 1;
  } else if ((lhs_result != 0) && (rhs_result == 0)) {
    return -1;
  } else {
    return 0;
  }
}
struct list *interpreter_sort_by(struct interpreter *interpreter,
                                 struct ast *ast) {
  struct list *nodes =
      unwrap_null(interpreter_nodes(interpreter, ast->sort_by.nodes));
  list_sort(&nodes);
  list_dedup_shallow(nodes);
  interpreter_sort_by_cmp_key = ast->sort_by.key;
  interpreter_sort_by_cmp_key_length = ast->sort_by.key_length;
  list_sort_custom(
      &nodes, (int (*)(const void *, const void *))interpreter_sort_by_cmp);
  return nodes;
}
struct list *interpreter_has_property(struct interpreter *interpreter,
                                      struct ast *ast) {
  if (ast->type != AST_HAS_PROPERTY) {
    return NULL;
  }
  struct list *vertices = list_create(object_type_vertex);
  struct aa_it *it = graph_iterator(interpreter->graph);
  while (it != NULL) {
    struct vertex *vertex = (struct vertex *)aa_it_object(it);
    if (vertex_has_property(vertex, ast->has_property.key,
                            ast->has_property.key_length)) {
      list_append(vertices, vertex);
    }
    it = aa_it_next(it);
  }
  return vertices;
}
struct list *interpreter_property_is(struct interpreter *interpreter,
                                     struct ast *ast) {
  if (ast->type != AST_PROPERTY_IS) {
    return NULL;
  }
  struct list *vertices = list_create(object_type_vertex);
  struct aa_it *it = graph_iterator(interpreter->graph);
  while (it != NULL) {
    struct vertex *vertex = (struct vertex *)aa_it_object(it);
    uint8_t *value;
    uint32_t value_length;
    int found =
        vertex_get_property(vertex, ast->property_is.key,
                            ast->property_is.key_length, &value, &value_length);
    if (found == 0) {
      if ((value_length == ast->property_is.value_length) &&
          (memcmp(value, ast->property_is.value, value_length) == 0)) {
        list_append(vertices, vertex);
      }
    }
    it = aa_it_next(it);
  }
  return vertices;
}
int interpreter_set_property(struct interpreter *interpreter, struct ast *ast) {
  if (ast->type != AST_SET_PROPERTY) {
    return -1;
  }
  struct list *vertices =
      interpreter_nodes(interpreter, ast->set_property.nodes);
  struct list_it *it;
  for (it = vertices->head; it != NULL; it = it->next) {
    struct vertex *vertex = (struct vertex *)it->object;
    if (vertex_set_property(
            vertex, ast->set_property.key, ast->set_property.key_length,
            ast->set_property.value, ast->set_property.value_length)) {
      return -1;
    }
  }
  list_delete_shallow(vertices);
  return 0;
}
int interpreter_update_node_id(struct interpreter *interpreter,
                               const struct ast *ast) {
  if (ast->type != AST_UPDATE_NODE_ID) {
    return -1;
  }
  struct list *vertices =
      interpreter_nodes(interpreter, ast->update_node_id.nodes);
  if (vertices->len != 1) {
    list_delete_shallow(vertices);
    return -1;
  }
  struct vertex *vertex = (struct vertex *)vertices->head->object;
  vertex->node_id = ast->update_node_id.node_id;
  list_delete_shallow(vertices);
  return 0;
}
int interpreter_set_successors(struct interpreter *interpreter,
                               const struct ast *ast) {
  if (ast->type != AST_SET_SUCCESSORS) {
    return -1;
  }
  struct list *nodes =
      unwrap_null(interpreter_nodes(interpreter, ast->set_successors.get));
  struct list *successors = unwrap_null(
      interpreter_nodes(interpreter, ast->set_successors.successors));
  struct list_it *it;
  for (it = nodes->head; it != NULL; it = it->next) {
    struct vertex *vertex = (struct vertex *)it->object;
    DEBUG("%lx", vertex->node_id);
    vertex_set_successors(vertex, successors);
  }
  list_delete_shallow(nodes);
  list_delete_shallow(successors);
  return 0;
}
int interpreter_set_predecessors(struct interpreter *interpreter,
                                 const struct ast *ast) {
  if (ast->type != AST_SET_PREDECESSORS) {
    return -1;
  }
  struct list *nodes =
      unwrap_null(interpreter_nodes(interpreter, ast->set_predecessors.get));
  struct list *predecessors = unwrap_null(
      interpreter_nodes(interpreter, ast->set_predecessors.predecessors));
  struct list_it *it;
  for (it = nodes->head; it != NULL; it = it->next) {
    struct vertex *vertex = (struct vertex *)it->object;
    vertex_set_predecessors(vertex, predecessors);
  }
  list_delete_shallow(nodes);
  list_delete_shallow(predecessors);
  return 0;
}
int interpreter_add_successors(struct interpreter *interpreter,
                               const struct ast *ast) {
  if (ast->type != AST_ADD_SUCCESSORS) {
    return -1;
  }
  struct list *nodes =
      unwrap_null(interpreter_nodes(interpreter, ast->set_successors.get));
  struct list *successors = unwrap_null(
      interpreter_nodes(interpreter, ast->set_successors.successors));
  struct list_it *it;
  for (it = nodes->head; it != NULL; it = it->next) {
    struct vertex *vertex = (struct vertex *)it->object;
    struct list_it *sit;
    for (sit = successors->head; sit != NULL; sit = sit->next) {
      struct vertex *successor = (struct vertex *)sit->object;
      vertex_push_successor(vertex, successor);
      vertex_push_predecessor(successor, vertex);
    }
  }
  list_delete_shallow(nodes);
  list_delete_shallow(successors);
  return 0;
}
int interpreter_add_predecessors(struct interpreter *interpreter,
                                 const struct ast *ast) {
  if (ast->type != AST_ADD_PREDECESSORS) {
    return -1;
  }
  struct list *nodes =
      unwrap_null(interpreter_nodes(interpreter, ast->set_predecessors.get));
  struct list *predecessors = unwrap_null(
      interpreter_nodes(interpreter, ast->set_predecessors.predecessors));
  struct list_it *it;
  for (it = nodes->head; it != NULL; it = it->next) {
    struct vertex *vertex = (struct vertex *)it->object;
    struct list_it *pit;
    for (pit = predecessors->head; pit != NULL; pit = pit->next) {
      struct vertex *predecessor = (struct vertex *)pit->object;
      vertex_push_predecessor(vertex, predecessor);
      vertex_push_successor(predecessor, vertex);
    }
  }
  list_delete_shallow(nodes);
  list_delete_shallow(predecessors);
  return 0;
}
int interpreter_remove_property(struct interpreter *interpreter,
                                const struct ast *ast) {
  if (ast->type != AST_REMOVE_PROPERTY) {
    return -1;
  }
  struct list *nodes =
      unwrap_null(interpreter_nodes(interpreter, ast->remove_property.nodes));
  struct list_it *it;
  for (it = nodes->head; it != NULL; it = it->next) {
    struct vertex *vertex = (struct vertex *)it->object;
    vertex_remove_property(vertex, ast->remove_property.key,
                           ast->remove_property.key_length);
  }
  list_delete_shallow(nodes);
  return 0;
}
struct list *interpreter_nodes(struct interpreter *interpreter,
                               const struct ast *ast) {
  if (ast->type != AST_NODES) {
    return NULL;
  }
  struct list *result = list_create(object_type_vertex);
  struct list_it *it;
  for (it = ast->nodes.nodes->head; it != NULL; it = it->next) {
    struct ast *ast = (struct ast *)it->object;
    switch (ast->type) {
    case AST_GET: {
      struct list *nodes = unwrap_null(interpreter_get(interpreter, ast));
      list_join(result, nodes);
      break;
    }
    case AST_GET_SUCCESSORS: {
      struct list *nodes =
          unwrap_null(interpreter_get_successors(interpreter, ast));
      list_join(result, nodes);
      break;
    }
    case AST_GET_PREDECESSORS: {
      struct list *nodes =
          unwrap_null(interpreter_get_predecessors(interpreter, ast));
      list_join(result, nodes);
      break;
    }
    case AST_NODES: {
      struct list *nodes = unwrap_null(interpreter_nodes(interpreter, ast));
      list_join(result, nodes);
      break;
    }
    case AST_WHERE_IN: {
      struct list *nodes = unwrap_null(interpreter_where_in(interpreter, ast));
      list_join(result, nodes);
      break;
    }
    case AST_WHERE_NOT_IN: {
      struct list *nodes =
          unwrap_null(interpreter_where_not_in(interpreter, ast));
      list_join(result, nodes);
      break;
    }
    case AST_FOR: {
      struct list *nodes = unwrap_null(interpreter_for(interpreter, ast));
      list_join(result, nodes);
      break;
    }
    case AST_HAS_PROPERTY: {
      list_join(result,
                unwrap_null(interpreter_has_property(interpreter, ast)));
      break;
    }
    case AST_PROPERTY_IS: {
      list_join(result, unwrap_null(interpreter_property_is(interpreter, ast)));
      break;
    }
    case AST_SORT_BY:
      list_join(result, unwrap_null(interpreter_sort_by(interpreter, ast)));
      return result;
    default:
      DEBUG("%s", ast_name(ast));
      panic("Invalid nodes type");
    }
  }
  list_sort(&result);
  list_dedup_shallow(result);
  return result;
}
struct list *interpreter_eval(struct interpreter *interpreter,
                              struct ast *ast) {
  int result = 0;
  switch (ast->type) {
  case AST_NODES:
    return interpreter_nodes(interpreter, ast);
  case AST_ADD:
    result = interpreter_add(interpreter, ast);
    break;
  case AST_REMOVE:
    result = interpreter_remove(interpreter, ast);
    break;
  case AST_UPDATE_NODE_ID:
    result = interpreter_update_node_id(interpreter, ast);
    break;
  case AST_SET_SUCCESSORS:
    result = interpreter_set_successors(interpreter, ast);
    break;
  case AST_SET_PREDECESSORS:
    result = interpreter_set_predecessors(interpreter, ast);
    break;
  case AST_ADD_SUCCESSORS:
    result = interpreter_add_successors(interpreter, ast);
    break;
  case AST_ADD_PREDECESSORS:
    result = interpreter_add_predecessors(interpreter, ast);
    break;
  case AST_DOT_GRAPH:
    result = graph_dot_graph(interpreter->graph);
    break;
  case AST_SET_PROPERTY:
    result = interpreter_set_property(interpreter, ast);
    break;
  case AST_REMOVE_PROPERTY:
    result = interpreter_remove_property(interpreter, ast);
    break;
  case AST_TRANSITIVE_CLOSURE:
    result = graph_transitive_closure(interpreter->graph);
    break;
  case AST_EXIT:
    interpreter->exited = 1;
    break;
  default:
    DEBUG("%u", ast->type);
    panic("Invalid AST type sent to interpreter_eval");
  }
  if (result == 0) {
    return list_create(object_type_vertex);
  } else {
    return NULL;
  }
}
struct list *interpreter_statement(struct interpreter *interpreter,
                                   struct ast *ast) {
  if (ast->type != AST_STATEMENT) {
    DEBUG("%s", ast_name(ast));
    panic("Non-statement ast sent to interpreter");
  }
  return interpreter_eval(interpreter, ast->statement.ast);
}
int interpreter_run(struct interpreter *interpreter, struct list *statements) {
  struct list_it *it;
  for (it = statements->head; it != NULL; it = it->next) {
    interpreter->scope = scope_create();
    struct list *vertices =
        interpreter_statement(interpreter, (struct ast *)it->object);
    if (vertices == NULL) {
      scope_cleanup(interpreter->scope);
      return -1;
    } else if (interpreter->exited) {
      list_delete(vertices);
      scope_cleanup(interpreter->scope);
      return 0;
    }
    struct list_it *vit;
    for (vit = vertices->head; vit != NULL; vit = vit->next) {
      struct vertex *vertex = (struct vertex *)vit->object;
      printf("0x%lx", vertex->node_id);
      fflush(stdout);
      struct aa_it *vpit = vertex_properties_iterator(vertex);
      while (vpit != NULL) {
        struct vertex_property *vp =
            (struct vertex_property *)aa_it_object(vpit);
        printf(" [%s=%s]", vp->key, vp->value);
        fflush(stdout);
        vpit = aa_it_next(vpit);
      }
      printf("\n");
      fflush(stdout);
    }
    list_delete_shallow(vertices);
    scope_cleanup(interpreter->scope);
  }
  return 0;
}