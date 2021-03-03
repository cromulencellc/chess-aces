#ifndef o_ast_HEADER
#define o_ast_HEADER

#include <stdint.h>

#include "container/list.h"
#include "container/vertex_identifier.h"
#include "o/token.h"

enum ast_type {
    AST_ADD,
    AST_REMOVE,
    AST_NODES,
    AST_GET,
    AST_FOR,
    AST_WHERE_IN,
    AST_WHERE_NOT_IN,
    AST_SET_SUCCESSORS,
    AST_SET_PREDECESSORS,
    AST_GET_PREDECESSORS,
    AST_GET_SUCCESSORS,
    AST_ADD_SUCCESSORS,
    AST_ADD_PREDECESSORS,
    AST_UPDATE_NODE_ID,
    AST_TOKEN,
    AST_TRANSITIVE_CLOSURE,
    AST_DOT_GRAPH,
    AST_EXIT,
    AST_STATEMENT,
    AST_SET_PROPERTY,
    AST_HAS_PROPERTY,
    AST_PROPERTY_IS,
    AST_REMOVE_PROPERTY,
    AST_SORT_BY,

    AST_END
};

struct ast_add_successors {
    struct ast * nodes;
    struct ast * successors;
};

struct ast_add_predecessors {
    struct ast * nodes;
    struct ast * predecessors;
};

struct ast_statement {
    struct ast * ast;
};

struct ast_sort_by {
    struct ast * nodes;
    uint8_t * key;
    uint32_t key_length;
};

struct ast_remove_property {
    struct ast * nodes;
    uint8_t * key;
    uint32_t key_length;
};

struct ast_set_property {
    struct ast * nodes;
    uint8_t * key;
    uint32_t key_length;
    uint8_t * value;
    uint32_t value_length;
};

struct ast_has_property {
    uint8_t * key;
    uint32_t key_length;
};

struct ast_property_is {
    uint8_t * key;
    uint32_t key_length;
    uint8_t * value;
    uint32_t value_length;
};

struct ast_update_node_id {
    struct ast * nodes;
    uint64_t node_id;
};

struct ast_add {
    uint64_t node_id;
};

struct ast_remove {
    uint64_t node_id;
};

struct ast_nodes {
    /* A list of type <parser/o/ast> */
    struct list * nodes;
};

struct ast_where_in {
    struct ast * nodes;
    struct ast * filter;
};

struct ast_where_not_in {
    struct ast * nodes;
    struct ast * filter;
};

struct ast_get {
    struct vertex_identifier * vi;
};

struct ast_for {
    struct vertex_identifier * vi;
    struct ast * get;
    struct ast * body;
};

struct ast_set_successors {
    struct ast * get;
    struct ast * successors;
};

struct ast_set_predecessors {
    struct ast * get;
    struct ast * predecessors;
};

struct ast_get_predecessors {
    struct ast * get;
};

struct ast_get_successors {
    struct ast * get;
};

extern const struct object * object_type_ast;

struct ast {
    const struct object * object;
    enum ast_type type;
    union {
        struct ast_statement statement;
        struct ast_add add;
        struct ast_remove remove;
        struct ast_nodes nodes;
        struct ast_get get;
        struct ast_for for_;
        struct ast_where_in where_in;
        struct ast_where_not_in where_not_in;
        struct ast_set_successors set_successors;
        struct ast_set_predecessors set_predecessors;
        struct ast_get_predecessors get_predecessors;
        struct ast_get_successors get_successors;
        struct ast_add_successors add_successors;
        struct ast_add_predecessors add_predecessors;
        struct ast_update_node_id update_node_id;
        struct token * token;
        struct ast_set_property set_property;
        struct ast_has_property has_property;
        struct ast_property_is property_is;
        struct ast_remove_property remove_property;
        struct ast_sort_by sort_by;
    };
};


struct ast * ast_create(enum ast_type type);
void ast_delete(struct ast * ast);
struct ast * ast_copy(const struct ast * ast);

const char * ast_name(const struct ast * ast);

/* All of these functions will take owership of what is passed to them */
struct ast * ast_create_statement(struct ast * ast);
struct ast * ast_create_add(uint64_t node_id);
struct ast * ast_create_remove(uint64_t node_id);
struct ast * ast_create_get(struct vertex_identifier * vi);
struct ast * ast_create_for(
    struct vertex_identifier * vi,
    struct ast * get,
    struct ast * body
);
struct ast * ast_create_where_in(struct ast * nodes, struct ast * filter);
struct ast * ast_create_where_not_in(struct ast * nodes, struct ast * filter);
struct ast * ast_create_nodes(struct ast * node);
void ast_nodes_append(struct ast * ast, struct ast * node);
struct ast * ast_create_set_successors(
    struct ast * get,
    struct ast * successors
);
struct ast * ast_create_set_predecessors(
    struct ast * get,
    struct ast * predecessors
);
struct ast * ast_create_update_node_id(struct ast * nodes, uint64_t node_id);
struct ast * ast_create_get_predecessors(struct ast * get);
struct ast * ast_create_get_successors(struct ast * get);
struct ast * ast_create_add_successors(
    struct ast * nodes,
    struct ast * successors
);
struct ast * ast_create_add_predecessors(
    struct ast * nodes,
    struct ast * predecessors
);
struct ast * ast_create_transitive_closure();
struct ast * ast_create_dot_graph();
struct ast * ast_create_exit();
struct ast * ast_create_token(struct token * token);
struct ast * ast_create_set_property(
    struct ast * nodes,
    const uint8_t * key,
    uint32_t key_length,
    const uint8_t * value,
    uint32_t value_length
);
struct ast * ast_create_has_property(const uint8_t * key, uint32_t key_length);
struct ast * ast_create_property_is(
    const uint8_t * key,
    uint32_t key_length,
    const uint8_t * value,
    uint32_t value_length
);
struct ast * ast_create_sort_by(
    struct ast * nodes,
    const uint8_t * key,
    uint32_t key_length
);
struct ast * ast_create_remove_property(
    struct ast * nodes,
    const uint8_t * key,
    uint32_t key_length
);

#endif