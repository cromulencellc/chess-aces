#ifndef aa_tree_HEADER
#define aa_tree_HEADER

#include "object.h"
#include <stdint.h>

extern const struct object * object_type_aa_tree;
extern const struct object * object_type_aa_node;

enum it_state {
    VISITED_SELF, /* We have not visited any children */
    VISITED_LEFT, /* We have visited children on the left */
    VISITED_RIGHT /* We have visited children on the right */
};

struct aa_it {
    const struct object * object;
    struct aa_it * parent;
    enum it_state state;
    struct aa_node * node;
};

struct aa_node {
    const struct object * aa_node_object;
    int64_t level;
    void * object;
    struct aa_node * left;
    struct aa_node * right;
};

struct aa_tree {
    const struct object * object;
    const struct object * type;
    struct aa_node * root;
};


struct aa_tree * aa_tree_create(const struct object * object);
void aa_tree_delete(struct aa_tree * tree);
struct aa_tree * aa_tree_copy(const struct aa_tree *);

int aa_tree_insert(struct aa_tree * tree, void * object);
void * aa_tree_fetch_ref(struct aa_tree * tree, const void * object);
int aa_tree_remove(struct aa_tree * tree, const void * object);
uint64_t aa_tree_num_items(const struct aa_tree * aa_tree);

struct aa_node * aa_node_create(void * object);
void aa_node_delete_(struct aa_node * aa_node);
struct aa_node * aa_node_copy(const struct aa_node * aa_node);

struct aa_node * aa_node_skew(struct aa_node * node);
struct aa_node * aa_node_split(struct aa_node * node);
struct aa_node * aa_node_insert(struct aa_node * node, void * object);
void * aa_node_fetch_ref(struct aa_node * node, const void * object);
struct aa_node * aa_node_decrease_level(struct aa_node * node);
struct aa_node * aa_node_successor(struct aa_node * node);
struct aa_node * aa_node_predecessor(struct aa_node * node);
struct aa_node * aa_node_remove(
    struct aa_node * node,
    int already_deleted,
    const void * object
);
uint64_t aa_node_num_items(const struct aa_node * node);


struct aa_it * aa_it_create();
/** This only cleans up the current iterator, and does not recurse to clean up
 * the iterator chain */
void aa_it_delete(struct aa_it *);
struct aa_it * aa_it_copy(const struct aa_it *);

/** This recurses the iterator chain and cleans everything up */
void aa_it_cleanup(struct aa_it *);

struct aa_it * aa_tree_iterator(struct aa_tree *);
struct aa_it * aa_it_next(struct aa_it *);
void * aa_it_object(struct aa_it *);

#endif