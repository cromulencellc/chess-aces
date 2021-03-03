#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "container/list.h"
#include "container/test_item.h"
#include "runtime/interpreter.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "parser/o/ast.h"
#include "rust.h"

void clean_shutdown() {
    exit(0);
}

void test_list_sort() {
    printf("%s\n", __FUNCTION__);
    struct list * list = list_create(object_type_test_item);

    list_sort(&list);

    list_append(list, test_item_create(3));
    list_append(list, test_item_create(4));
    list_append(list, test_item_create(1));
    list_append(list, test_item_create(8));
    list_append(list, test_item_create(2));
    list_append(list, test_item_create(7));
    list_append(list, test_item_create(5));
    list_append(list, test_item_create(6));

    list_sort(&list);

    struct list_it * it = list->head;

    assert(((struct test_item *) it->object)->num == 1);
    it = it->next;
    assert(((struct test_item *) it->object)->num == 2);
    it = it->next;
    assert(((struct test_item *) it->object)->num == 3);
    it = it->next;
    assert(((struct test_item *) it->object)->num == 4);
    it = it->next;
    assert(((struct test_item *) it->object)->num == 5);
    it = it->next;
    assert(((struct test_item *) it->object)->num == 6);
    it = it->next;
    assert(((struct test_item *) it->object)->num == 7);
    it = it->next;
    assert(((struct test_item *) it->object)->num == 8);
    it = it->next;

    object_delete(list);
}


void test_list_dedup() {
    printf("%s\n", __FUNCTION__);
    struct list * list = list_create(object_type_test_item);

    list_dedup(list);

    list_append(list, test_item_create(1));
    list_append(list, test_item_create(2));
    list_append(list, test_item_create(3));
    list_append(list, test_item_create(3));
    list_append(list, test_item_create(4));

    list_dedup(list);

    struct list_it * it = list->head;

    assert(((struct test_item *) it->object)->num == 1);
    it = it->next;
    assert(((struct test_item *) it->object)->num == 2);
    it = it->next;
    assert(((struct test_item *) it->object)->num == 3);
    it = it->next;
    assert(((struct test_item *) it->object)->num == 4);

    object_delete(list);
}


void test_aa_tree() {
    printf("%s\n", __FUNCTION__);
    struct aa_tree * tree = aa_tree_create(object_type_test_item);

    aa_tree_insert(tree, test_item_create(1));
    aa_tree_insert(tree, test_item_create(5));
    aa_tree_insert(tree, test_item_create(2));
    aa_tree_insert(tree, test_item_create(6));
    aa_tree_insert(tree, test_item_create(3));
    aa_tree_insert(tree, test_item_create(4));
    aa_tree_insert(tree, test_item_create(7));
    aa_tree_insert(tree, test_item_create(0));

    unsigned int i;
    for (i = 0; i < 8; i++) {
        struct test_item * needle = test_item_create(i);
        assert(((struct test_item *) aa_tree_fetch_ref(tree, needle))->num == i);
        object_delete(needle);
    }
    
    object_delete(tree);
}


int main() {
    test_list_sort();
    test_list_dedup();
    test_aa_tree();
    return 0;
}