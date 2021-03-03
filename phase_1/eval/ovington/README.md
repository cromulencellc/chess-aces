# ovington

Database, with LISP-based query language, special data-load protocol, and
on-disk storage.

## concept

Submit data, get GUID of table.

Write queries in LISP (but with easier-to-parse representation).

### "oqtopus" query language

Ovington
Query
Transmission
Organized
Program
Unit
Serialization

LISP implementation and easy-to-parse wire format. On-wire, we have:

* `sint64`, 64-bit integers
* `float64`, 64-bit floats
* `stringz`, null-terminated strings
* `keyword`, length-prefixed string but it's treated as a keyword
* `openlist`, `(`
* `closelist`, `)`

In addition, we have internally:

* Cell, [a cons-cell][conscell], an important building block for LISP concepts
* NativeLambda, connects oqtopus to c++ functions
* RowLambda, a user-provided LISP function evalued per-row in a collection
* nullptr

[conscell]: https://en.wikipedia.org/wiki/Cons

The empty cell (i.e. the cell that has an [`car` and `cdr`][carcdr] of `nullptr`)
is false-y, pretty much everything else is true-thy.

[carcdr]: https://en.wikipedia.org/wiki/CAR_and_CDR

#### how oqtopus works ("the oqtopus screed")

Oqtopus is heavily influenced by Norvig's
["(How to Write a (Lisp) Interpreter (in Python))"][lispy].
The reason for writing it mostly comes from Yegge's
["Rich Programmer Food"][olivegarden], although that can be summed up as
"I wanted to."  The Wikipedia articles about [cons-cell][conscell]s and
[CAR and CDR][carcdr] are also useful reads.

[lispy]: http://www.norvig.com/lispy.html
[olivegarden]: https://steve-yegge.blogspot.com/2007/06/rich-programmer-food.html

Filenames in this section are under `challenge/src` or the corresponding variant
directory. Class and function names in this section are case-sensitive. In
particular, there's both an `Oqtopus` class in `oqtopus.cpp`, and an `oqtopus`
namespace in the `oqtopus/` subdirectory.

In `main.cpp`, Ovington either accepts a network connection or uses stdio to
initialize a `Connection` object, and then calls `Connection::service()` on that
object.

A `Connection` object has an `Oqtopus` state object as a member variable. This
state object is basically a couple flags that a `Connection` needs to care
about (`wants_data_load`, `wants_disconnect`) that can be set by oqtopus
built-ins, and a default `oqtopus::Environment`. More on the `Environment`
concepts later (it’s all very tangly and referential)

`Connection::service()` (`connection.cpp`), in a loop, does something with the
connection based on the `ConnectionState`:
* `ConnectionState::disconnect`: returns out of the loop
* `ConnectionState::data`: runs `DataLoad::do_dl()` to read data in a format
  from the client that has less overhead than oqtopus
* `ConnectionState::oqtopus`: has the `Oqtopus` object consume
  (read, evaluate, print-results-of) a statement

The third one is the most interesting and complex.

Oqtopus has a top level `Value` data type, which is one of:
* `Sint64`
* `Float64`
* `Stringz`: for data that oqtopus treats like a bunch of bytes
* `Keyword`: function names, basically
* `Openlist`
* `Closelist`
* `Cell`: a "cons cell," two `ValuePtrs`, important building block for LISP
* `NativeLambda`: a function pointer to a c++ function
* `RowLambda`: a user-provided Oqtopus function

A `ValuePtr` is a reference-counted pointer to a `Value`. The reference
counting is done with `std::shared_ptr` from the C++ standard library.

An `oqtopus::Environment` is a mapping of strings/keywords to `ValuePtrs`.
The default environment is basically the built-in `NativeLambda`s
implementing `+`, `-`, `count-rows`, etc.

Here’s how Oqtopus evaluation works:

1. When the `Connection` is created, before any traffic is processed, an
  `oqtopus::Environment` is initialized as part of the `Oqtopus` member of the
  `Connection`.
2. `Oqtopus::consume` (`oqtopus.cpp`) has `oqtopus::ValueVec`’s constructor
  (`oqtopus/valuevec.cpp`) parse a vector of tokens off the wire. `ValueVec`
  counts `openlist` and `closelist` tokens, and stops parsing after the last
  `closelist`. For example, the statement `(+ 1 (* 2 3))` would become
  `[openlist,
  keyword(+), sint64(1), openlist, keyword(*), sint64(2), sint64(3), closelist,
  closelist]`
3. `Oqtopus::consume` has the token vector `resolve()` itself into an
  expression, i.e. instead of a linear vector of tokens, become a nested
  S-expression. The above statement would become basically a parse tree built
  out of cons `Cell`s
4. `Oqtopus::consume` checks to see if it’s the single keyword `data-load`. If
  so, it sets the `want_data_load` flag to true so the connection stops being
  an oqtopus.
5. `Oqtopus::consume` calls `oqtopus::eval` (`oqtopus/eval.cpp`, line 142)
  with the expression `ValuePtr`
6. `oqtopus::eval` (or `eval` for short) tries to cast the the expression to a
  `Keyword`. If it is a keyword, it fetches the value from the environment and
  returns it. This is used for variable substitution, like the `true` and
  `false` keywords in the default environment.
7. `eval` checks to see if the expression is an atom (a number or string). If
  so, it returns it.
8. `eval` checks to see if the expression is a `Cell` (line 157); i.e. a
  linked-list containing a function call or special form. If not, it errors out.
9. `eval` grabs the first part of the expression (the “car”) and tries to make
  it a `Keyword`. If it’s not a keyword, it errors out. (lines 161-170)
10. `eval` now checks for some special forms that cannot be functions at lines
  172-181. `quote` simply returns its arguments verbatim without evaluation.
  `if` only evaluates some of its arguments. `row-lambda` doesn’t evaluate its
  arguments yet. `set` manipulates the environment. If one of these special
  forms is evaluated, we’re done.
11. Finally! It’s time to call a function.
12. We use `eval(args->car(), env)` to turn the `Keyword` representing the
  function name into a callable (this will be a `ValuePtr` to a `NativeLambda`)
  (recursively calling eval to look up the function in the environment like in
  step 6 above)
13. We use `eval_args(args->cdr(), env)` to eval all the arguments (turning
  `(+ 1 (* 2 3))` into `(+ 1 6)`)
14. `eval` `eval_proc`s the callable function and the arguments for it and
  returns the result.
15. In `eval_proc` (`oqtopus/eval.cpp`, lines 34-35), we cast the callable to a
  `NativeLambda`…
16. and use that `NativeLambda`’s `operator()` method (implemented by the
  `std::function` library class that `NativeLambda` inherits from) to actually
  invoke it (`oqtopus/eval.cpp`, line 36).

Evaluating LISP is a tangly recursive process, but it's an important process,
as it's very similar to

### data-load protocol

Part of the "ODBC" (Ovington Data Base Classes) suite, this saves a bunch
of bandwidth over how you'd do it with oqtopus.

It stores into three files, .defs, .fixeds, .strings. .defs contains
column definitions, fixeds contains fixed-width values,
strings contains variable-size values.

The "ODBC" in this challenge is unrelated to the "Open DataBase Connectivity"
APIs.

## polling and other testing

Most activities in the poller should have documentation or be very
self-descriptively-named. Deleting `/data` after each run is recommended
but not strictly necessary as long as `/dev/urandom` has good-quality
output.

Running the poller with a filename in the `DUMP_EXAMPLE` environment
variable will also dump the poller's traffic into a file suitable as
an example for a fuzzer.

The `NO_FILESYSTEM` compile-time flag (i.e. `-DNO_FILESYSTEM` as an
argument to `clang++`) removes file-touching code to make it easier to
fuzz; you won't have to blow away `/data` this way.

## variants and vulnerabilities

There are two vulnerabilities and two proofs-of-vulnerability.

The NativeLambda objects have `cast_float` implemented, which treats the address
of the C++ function they contain as a float64 and returns it. It's possible to
exfiltrate this by using a NativeLambda as the second or later argument to a
numeric function like `+`. The address is significant as ovington is position-
independent and as such function locations are privileged. The pov in the
`pov_nl_cast_to_float` directory demonstrates this flaw. The patch is to
simply not allow casting function pointers to floats.

Most oqtopus objects have an `inspect` method used to produce debug output:

```
[INFO] src/oqtopus.cpp:15 void Oqtopus::consume(int, int): + 0.000000 +
[INFO] src/oqtopus/eval.cpp:186 oqtopus::ValuePtr oqtopus::eval(oqtopus::ValuePtr, oqtopus::Environment &): NativeLambda(0x3e48)
[INFO] src/oqtopus/eval.cpp:188 oqtopus::ValuePtr oqtopus::eval(oqtopus::ValuePtr, oqtopus::Environment &): 0.000000 NativeLambda(0x3e48)
```

When debugging, it's really convenient to be able to know which NativeLambda
object calls which function in the executable. Unfortunately, this information
shouldn't be available over the network, but the `inspect` function in oqtopus
calls the `inspect` method. The pov in the `pov_inspect_nl` directory
demonstrates this. My patch as-is masks off the most-significant bits of the
NativeLambda's function address, making is mostly-useless. Other acceptable
patches might remove the function address entirely from the NativeLambda's
`inspect` output, or something else of the sort.

Because there are two patches, there are four total variants. `fully_patched`
makes `pov_inspect_nl` return a useless address and `pov_nl_cast_to_float`
fail.

```
root@94553710c6b9:/pov# HOST=ovington_challenge_run_26 PORT=3015 ruby ../pov_inspect_nl/pov.rb
TIMEOUT=5
ADDRESS=ae48
root@94553710c6b9:/pov# HOST=ovington_challenge_run_26 PORT=3015 ruby ../pov_nl_cast_to_float/pov.rb
TIMEOUT=5
FATAL: Expected address, got nil
```

`unpatched` returns valid addresses from both povs.

```
root@94553710c6b9:/pov# HOST=ovington_challenge_run_26 PORT=3015 ruby ../pov_inspect_nl/pov.rb
TIMEOUT=5
ADDRESS=55be70e51e48
root@94553710c6b9:/pov# HOST=ovington_challenge_run_26 PORT=3015 ruby ../pov_nl_cast_to_float/pov.rb
TIMEOUT=5
ADDRESS=55be6ec60d90
```

`variant_1` works with `pov_inspect_nl` but `pov_nl_cast_to_float` fails.

```
root@94553710c6b9:/pov# HOST=ovington_challenge_run_26 PORT=3015 ruby ../pov_inspect_nl/pov.rb
TIMEOUT=5
ADDRESS=558621c26e48
root@94553710c6b9:/pov# HOST=ovington_challenge_run_26 PORT=3015 ruby ../pov_nl_cast_to_float/pov.rb
TIMEOUT=5
FATAL: Expected address, got nil
```

`variant_2` works with `pov_nl_cast_to_float`, and `pov_inspect_nl` returns
a uselessly vague piece of address.

```
root@94553710c6b9:/pov# HOST=ovington_challenge_run_26 PORT=3015 ruby ../pov_inspect_nl/pov.rb
TIMEOUT=5
ADDRESS=3e48
root@94553710c6b9:/pov# HOST=ovington_challenge_run_26 PORT=3015 ruby ../pov_nl_cast_to_float/pov.rb
TIMEOUT=5
ADDRESS=564349424d90
```
