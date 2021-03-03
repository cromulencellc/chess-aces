# york

This document discusses the architecture and vulnerabilities of this CHESS
challenge. It should not be shared with TA1, TA2, or TA5 performers prior
to Phase 1 evaluations.

York is a web server built using [webmachine][webmachine] techniques. To map REST
concepts on to HTTP concepts more effectively, the `Connection` class asks a
`Resource` class a bunch of questions to build a response, instead of the
Rack-style app more commonly seen (in which a responder builds its own
response).

[webmachine]: https://github.com/webmachine/webmachine/wiki

The resources available are for serving static files
(`http::resources::Static`) and orders from an e-commerce site
(`http::resources::Orders`).

The orders resource provides an index that shows order numbers,
and order views that either show just the order number, or given a postal code,
full order information, including payment card numbers ("Primary Account
Number," or "PAN").

## YAML Files

York Application Marshalling Library is basically a database system. It bears
no relation (beyond the name) to https://yaml.org/ .

### Building Them

YAML files for `base_data` and CSV files for `poller/data` are generated using
MS Excel and the `priv/csv_to_yaml.rb` script.

Names and addresses generated by: https://www.fakenamegenerator.com

Primary Account Numbers (i.e. credit card numbers) are generated using
`priv/ccgen.js` ; this tool is forked from the GPL licensed
https://github.com/VRMink/credit-card-generator and modified to output even
more invalid but Luhn-correct account numbers (which are then hand-copied into
`priv/orders.xlsx`).

## YAML Queries

YAML queries are expressed using a stack-based language, with mostly one-byte
opcodes. These opcodes are listed in `yaml/opcode.hpp`:

```cpp
    enum class Code : char {
                            push_str = '"',
                            deref = '*',
                            eq = '=',
                            like = '~',
                            not_op = '!',
                            and_op = '&',
                            or_op = '|',
                            xor_op = '^',
                            push_true = 't',
                            push_false = 'f'
    };
```

The `push_str` opcode scans the input program up to a null byte (presented as 
`\0` in this document), and pushes
that to the stack. `deref` takes the top-most string on the stack, and replaces
it with the value from that named column in the current row.

The OrderFinder class in `http/resources/orders.cpp` builds queries for
the three kinds of scenario:

* finding all orders: this is the query `t` which simply pushes a `true`
  boolean to the stack, matching every order
* finding an order by number: this is the query
  `"order number\0*"11111111-1111-1111-1111-111111111111\0=`. This query
  1. pushes "order number" to the stack
  2. derferences it, i.e. gets the "order number" value from the current row
  3. pushes the order number (an all-ones UUID in this case) to the stack
  4. pops the two strings from the stack, and if they're equal, pushes a
     `true` to the stack, or a `false` if they're not equal.
* finding an order by number and postal code: this is the query (line breaks
  inserted for readability; this is not syntactically correct):
  ```
  "shipping address\0*"33133\0~
  "billing address\0*"33133\0~
  |
  "order number\0*"
  11111111-1111-1111-1111-111111111111
  \0=
  &
  ```
  By (visual) line, this query:
  1. pushes "shipping address" to the stack, dereferences it, pushes the
     ZIP code "33133" to the stack, uses `~` ("like") to pop the needle
     ("33133") and the haystack (the row's "shipping address"), and pushes
     a `true` or `false` if the needle is in the haystack
  2. similar to the previous line, it checks if the ZIP "33133" is in the
     billing address
  3. `|`, the boolean OR operation, pops the two booleans (first billing
     match, then shipping match), and ORs them together
  4. put the current row's "order number" on the stack, and start a string
  5. The user-provided order number
  6. Terminate the order number with a null `\0`, and check if it's equal
     the current row's
  7. `&`, the boolean AND, pops the order number equality result and the
    shipping match OR billing match result, and pushes that final result
    to the stack

## weakness and pov

The weakness is code/data injection in the `Orders` resource. The order
and zip strings are not properly escaped, allowing something akin to a SQL
injection attack to expose order PANs without knowing their postal code.

The included PoV asks the orders index the same thing as any normal poller
request: for a list of all the order numbers (which are UUIDs). Then, for
each order, it does a YAML injection, giving an order number of:
`11111111-1111-1111-1111-111111111111\0="\0"` and a placeholder postal code
of `11111`. When meshed into the query for finding orders by number & postal
code, it looks like this (line breaks inserted for readability):
```
"shipping address\0*"11111\0~
"billing address\0*"11111\0~
|
"order number\0*"
11111111-1111-1111-1111-111111111111\0="\0"
\0=
&
```
By (visual) line, this query:
1. pushes "shipping address" to the stack, dereferences it, pushes the
    ZIP code "33133" to the stack, uses `~` ("like") to pop the needle
    ("33133") and the haystack (the row's "shipping address"), and pushes
    a `true` or `false` if the needle is in the haystack
2. similar to the previous line, it checks if the ZIP "33133" is in the
    billing address
3. `|`, the boolean OR operation, pops the two booleans (first billing
    match, then shipping match), and ORs them together
4. put the current row's "order number" on the stack, and start a string
5. The user-provided order number, terminated with a user-provided null,
   and a check if it's equal to the current row's order number from the
   previous line. It also pushes an empty string, and the `push_str`
   opcode to match the one in the previous line 
6. Terminate the second user-provided empty string with a null `\0`, and check
   if it's equal to the other user provided empty string
7. `&`, the boolean AND, pops the empty string equality result and the order
   number equality result and pushes that final result
   to the stack. The (almost certainly false) shipping match OR billing match
   result lingers on the stack.

## patching it

Patching out showing the PAN at all isn't workable, since the poller expects
to see it. Properly escaping the query (for example, by truncating them to
the first null byte, if one exists) is the acceptable solution.