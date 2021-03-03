#ifndef scope_HEADER
#define scope_HEADER

#include "object.h"

#include "container/aa_tree.h"
#include "container/list.h"

extern const struct object * object_type_variable;

struct variable {
    const struct object * object;
    char * label;
    /* container/list of the <graphdb/vertex> */
    struct list * vertices;
};


struct variable * variable_create(const char * label);
void variable_delete(struct variable * variable);
struct variable * variable_copy(const struct variable *);
int variable_cmp(const struct variable *, const struct variable *);


struct scope {
    struct scope * parent;
    struct aa_tree * variables;
};

struct scope * scope_create();
void scope_delete(struct scope *);
void scope_cleanup(struct scope *);
struct scope * scope_push(struct scope * scope);
struct scope * scope_pop(struct scope * scope);
int scope_set_variable(
    struct scope * scope,
    const char * label,
    struct list * vertex_objects
);
struct list * scope_get_variable_ref(struct scope * scope, const char * label);

#endif