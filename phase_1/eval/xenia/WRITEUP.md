# Xenia Bug & Exploit Writeup

We need background into select portions of Xenia's internal functioning before
we can explain the bug, and how to exploit it.

## Objects

Almost everything in Xenia is an, "Object." An object in Xenia begins with a
pointer to a vtable of type `struct object`.

```c
struct object {
    object_delete_f delete;
    object_copy_f copy;
    object_cmp_f cmp;
};
```

Containers work over objects. For example, let's say we want to create a list of
type `ou64` (object unsigned 64-bit). We would first create the list, and
declare the type of object we are going to add to the list.

```c
struct list * ou64_list = list_create(OBJECT_TYPE_OU64);
```

No we can create an `ou64` and add it to the list.

```c
struct ou64 * new_ou64 = ou64_create(0xdeadbeef);
list_append(ou64_list, new_ou64);
```

When this happen, the list will verify the type of object being added to it. It
does this by comparing the first 8-byte value in the passed value to the type
pointer it was given during initialization. These must match. This gives us
statically declared, dynmically checked typing of containers.

Now we can copy this list:

```c
struct list * new_list = object_copy(ou64_list);
```

This will use the object vtables to copy all of the underlying data structures.
We can also delete lists this way:

```c
object_delete(new_list);
```

It's important to note that objects are normally checked when new objects are
introduced to containers, such as when object are added, or searched for, but
once an object is in the container, the container does not continually verify
the types of objects it holds within it.

This type checking is implemented on most containers. Relevant to us, it is also
implemented on `aa_tree`.

## Graphs

### The graph

Graphs are fairly simple, and are wrappers around an `aa_tree` holding vertices.

```c
struct graph {
    const struct object * object;
    struct aa_tree * vertices;
};
```

### Vertices

This is the type of a vertex in our graph.

```c
struct vertex {
    const struct object * object;
    uint64_t node_id;
    struct aa_tree * properties;
    struct vertex ** successors;
    struct vertex ** predecessors;
    uint16_t size_of_successors;
    uint16_t num_successors;
    uint16_t size_of_predecessors;
    uint16_t num_predecessors;
};
```

Vertices have arrays of pointers directly to other vertices. This is how edges
are tracked in the graph.

When we gather successors/predecessors of vertices, these pointers are added to
a list of type Vertex, which means all elements of these arrays must point to
valid Vertices.

Additionally, these lists are de-duped by `interpreter.c:interpreter_nodes`.
This is done because we can create queries that gather vertices, and we could
potentially get many duplicate vertices. If we have `1 -> 3` and `2 -> 3` and we
want all of the successors to both `1` and `2`, we only want the result vertex,
`3`, once.

## The bug

We can add the same successor or predecessor to the same vertex multiple times.
However, when we fetch successors/predecessors, those results will be
de-duplicated.

When we delete a vertex from the graph, that vertex will go to all of its
predecessors and successors, and remove itself from those vertices. However,
this list is also de-duplicated. If we have `1 -> 2` twice, and we delete `1`,
only the first instance of `1` as a predecessor of `2` will be deleted from
`2`'s list of predecessors. We have a dangling pointer to freed space which
expects an Object of type vertex.

We will call this vertex `1`, the vertex we are freeing, the, "Malicously freed
vertex."

In this case, if we wanted to get our maliciously freed vertex, we would ask
for the predecessors of `2`.

## Exploiting the bug

To exploit the bug, we need to place a new vertex we control into the space of
a malicously freed vertex. To help us do this, the pov has a function called
`find_overflow`. This tries multiple patterns of allocations and frees, and then
returns to us a buffer and offset which will take the place of our malicously
freed vertex.

### Leaking

We now have to craft a very special vertex. Vertices have properties, which are
mapping of keys to values, where keys and values are byte arrays. We will create
a vertex with a property, where the value of that property points to a location
of our choosing. We will then get that vertex, which will return to us the value
pointed to by that property.

To do this we must create an `aa_tree` for the properties, an `aa_tree_node` to
hold our property, and a `property`. These must all point to each other
correctly. Fortunately, our input buffer is at a known, static address. While
reading in input, input will be read until a newline is encountered. Also, the
lexer supports comments, so all input after a comment character, `;`, will be
ignored until a newline. _We can insert null-bytes here_. As long as we are free
of `;` and `\n`, we are good to put whatever data we want at a known address.

We craft a very special `aa_tree` inside this input buffer, and then drop in a
fake Vertex which points to it. We fetch this vertex, which will leak out our
target address. We do this to get a pointer at a known offset into libc. From
this pointer, we can calculate a pointer to `system`.

### Exploiting

The `object_cmp` function is a little weird. We might expect the function to
look like this (where we call `cmp` on the `lhs`):

```c
int object_cmp(const void * lhs, const void * rhs) {
    return (*((struct object **) lhs))->cmp(lhs, rhs);
}
```

When in fact we have this (we call `cmp` on the `rhs`):

```c
int object_cmp(const void * lhs, const void * rhs) {
    return (*((struct object **) rhs))->cmp(lhs, rhs);
}
```

Because both objects should always be the same, it does not matter which object
we dereference for our `cmp` function pointer. However, this setup allows us to
control the value of the first argument passed to a function.

We also must familiarize ourselves with the algorithm for deleting an object
from an aa_tree: [Wikipedia article on AA tree node deletion](https://en.wikipedia.org/wiki/AA_tree#Deletion).
Let's say we have a graph that looks like this:

```
  A
 / \
B   C
   /
  D
```

_`B` is irrelevant in this example, just for illustrative purposes_

If we want to delete `A`, we can replace `A` with `D`, and then try to delete
`D`. This allows us to delete a leaf node, greatly simplifying the process.

We want to make a call to system where we control the first argument to the
call. In order to do this, we will need to compare two arbitrary objects in an
AA Tree. However, Xenia's AA Tree will type check any node sent to it. In our
example graph, we can create two malicous vertices at `A` and `C`. We can then
attempt to delete vertex `A`. When this happens, `A` will be replaced with `D`,
and `A` will become the vertex we are trying to delete. Let's break this down
visually.

```
  A <- Malicious vertex
 / \
B   C <- Malicious vertex
   /
  D <- Irrelevant, but must exist
```

We introduce the `NEEDLE` vertex into the mix. This is a valid vertex created by
Xenia, not the attacker.

```
  A <= NEEDLE->cmp(A, NEEDLE) // true
 / \
B   C
   /
  D
```

We now replace `A` with `D`, `D` with `A`, and we begin searching for `A`.

```
  D
 / \
B   C <= C->cmp(A, C)
   /
  A
```

We control `C` and `A` entirely. They do not need to be valid objects. We can
set the `cmp` function of `C` to point to `system`, and the `object` pointer in
`A` will instead hold the string `"/bin/sh\0"`

This is how the exploit works.

### Putting it together

We run `find_overflow` to find a pattern and offset which gives us control over
a maliciously freed vertex. This generally needs to be done once per libc
implementation.

We then leak out the address of a pointer into libc.

We then use that leaked pointer to find a pointer to `system`, and then craft a
series of malicious objects such that when a specific property is deleted from
our maliciously deleted vertex, a call to `system("/bin/sh\0")` is made instead.