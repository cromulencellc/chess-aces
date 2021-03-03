#include "scope.h"

#include <stdlib.h>
#include <string.h>

#include "rust.h"
#include "graphdb/vertex.h"

const static struct object variable_object = {
    (object_delete_f) variable_delete,
    (object_copy_f) variable_copy,
    (object_cmp_f) variable_cmp
};

const struct object * object_type_variable = &variable_object;


struct variable * variable_create(const char * label) {
    struct variable * variable =
        (struct variable *) malloc(sizeof(struct variable));
    
    variable->object = &variable_object;
    variable->label = strdup(label);
    variable->vertices = list_create(object_type_vertex_object);

    return variable;
}

void variable_delete(struct variable * variable) {
    list_delete(variable->vertices);
    free(variable->label);
    free(variable);
}

struct variable * variable_copy(const struct variable * variable) {
    struct variable * new_variable = variable_create(variable->label);

    list_delete(new_variable->vertices);
    new_variable->vertices = list_copy(variable->vertices);

    return new_variable;
}

int variable_cmp(const struct variable * lhs, const struct variable * rhs) {
    return strcmp(lhs->label, rhs->label);
}



struct scope * scope_create() {
    struct scope * scope = (struct scope *) malloc(sizeof(struct scope));
    scope->parent = NULL;
    scope->variables = aa_tree_create(object_type_variable);
    return scope;
}

void scope_delete(struct scope * scope) {
    aa_tree_delete(scope->variables);
    free(scope);
}

void scope_cleanup(struct scope * scope) {
    if (scope == NULL) {
        return;
    }
    scope_cleanup(scope->parent);
    scope_delete(scope);
}

struct scope * scope_push(struct scope * scope) {
    struct scope * new_scope = scope_create();
    new_scope->parent = scope;
    return new_scope;
}

struct scope * scope_pop(struct scope * scope) {
    if (scope == NULL) {
        return NULL;
    }
    struct scope * parent = scope->parent;
    scope_delete(scope);
    return parent;
}

int scope_set_variable(
    struct scope * scope,
    const char * label,
    struct list * vertex_objects
) {
    struct variable * variable = variable_create(label);

    /* If there is a variable already in there, get rid of it */
    aa_tree_remove(scope->variables, variable);

    list_join(variable->vertices, vertex_objects);

    return aa_tree_insert(scope->variables, variable);
}

struct list * scope_get_variable_ref(struct scope * scope, const char * label) {
    if (scope == NULL) {
        return NULL;
    }

    struct variable * variable = variable_create(label);

    struct variable * result = aa_tree_fetch_ref(scope->variables, variable);
    variable_delete(variable);
    if (result == NULL) {
        return scope_get_variable_ref(scope->parent, label);
    }

    return result->vertices;
}