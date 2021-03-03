Welcome to the Very Fast Graph Database (VFGD)! This early alpha release is
feature incomplete, but we're working on it!

VFGD Query Language
===================

All vertices in the VFGD are identified by a 64-bit integer.

To add vertices to your graph, use the add command, like so:

----- BEGIN EXAMPLE -----
add(1);
add(2);
add(3);
----- END EXAMPLE -----

You can set vertices as successors and predecessors of one another

----- BEGIN EXAMPLE -----
set_successors(get(1), [get(2), get(3)]);
set_successors(get(2), get(3));
----- END EXAMPLE -----

You can then query the graph like so:

----- BEGIN EXAMPLE -----
get_successors(get(1));
get_successors(get_successors(get(1)));
where get_successors(get(1)) in get_predecessors(get(3));
for x in get_successors(get(1)) get_predecessors(x);
----- END EXAMPLE -----

Thanks for trying out VFGD!