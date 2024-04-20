<img src="./doc/slang-dark.png" width="200">

# slang

A stack-based bytecode Vm written in C. It's a dynamically typed (_currently_), garbage-collected and object-oriented programming language with a syntax that is inspired by C#, JavaScript and Python.

## Roadmap for Version 1.0

### Syntax & Language Features

- [ ] Add nillish coalescing operator `??` e.g. `let x = [1] <newline> let v = x[1] ?? 0`
- [ ] String interpolation. C#-style `$"Hello {name}"` (**_See Challenge 16.1_**)
- [ ] Add `const` (**_See Challenge 22.3_**)
- [ ] Implement `Json` module
- [ ] Implement `Test` class / module with `Assert.that(expected, Is.equal_to(actual))`
- [ ] Implement `Set` class
- [ ] Implement `Set.add(Obj) -> Nil` as a builtin
- [ ] Implement `Set.del(Obj) -> Nil` as a builtin
- [ ] Implement `Seq(Set)` constructor
- [ ] Implement `Seq.sort(sortFn) -> Seq` as a builtin
- [ ] Implement `Seq.slice(Num?, Num?) -> Seq` as a builtin
- [ ] Implement `Seq[1..] -> Seq` as a builtin (slice)
- [ ] Implement `Seq[..-1] -> Seq` as a builtin (slice)
- [ ] Implement `Seq[1..-1] -> Seq` as a builtin (slice)
- [ ] Implement `Seq[-1] -> Obj` as a builtin
- [ ] Implement `Gc` module
- [ ] Implement `Gc collect() -> Nil` as a builtin
- [ ] Implement `Gc stats() -> Obj` as a builtin
- [ ] Implement `match` Statement. (**_See Challenge 23.1_**, on how to impl `switch`, that's a start.)
- [ ] Implement `nameof` keyword. E.g. `nameof(foo)` returns `"foo"`. (**_See Challenge 22.1_**)
- [ ] Implement `@memoize` decorator. How would this work? We would need be able to compare objects by their value instead of their reference (Stringification comes to mind - but that's slow). Maybe we can devise some kind of special hash function for this? E.g. for a seq, we could hash each element and then hash these hashes.
- [ ] Implement a way to add a doc string to functions and classes (managed code). This involves defining a new syntax for such a thing. Maybe attributes? (e.g. `@doc "This is a doc string"`)
- [ ] Implement iterators. Maybe a new builtin class. They should initialize these fields/methods: `__has_next`, `__next()`. They should use the `Obj`s prop_getter, \_setter and index_getter, \_setter functions. (Maybe they shouldn't return the internal fields though).
- [ ] Implement `for ... in ...;` loops

## Improvements

- [ ] Remove bounds checks for seq/str indexing. Just return `NIL_VAL` if out of bounds. At least for get-access. Set-access should still throw an error.
- [ ] Add an iterator to builtin classes `Class` and `Fn` which holds all the things you can 'get'. Use this in their `has` and `prop_getter` functions to make sure they are always in sync.
- [ ] Use something else instead of `rint`, because it's not very precise. See _num-to-str.spec.sl_ for an example.
- [ ] Closing over the loop variable. (**_See Challenge 25.2_**)
- [ ] Currently, `i++` behaves more like `++i` (Which we don't support). Fix it.
- [ ] Add `error` to the reserved words
- [ ] Make sure managed code classes do not override internal classes.
- [ ] Move exit codes to `common.h` and replace all magic numbers with them
- [ ] Add a guard in `compiler.c -> number()` to check for overflow.
- [ ] Align error messages. Some use `'` around names, or type names, some don't.
- [ ] Allow return stats without suffixed `;`
- [ ] Remove `OP_PRINT` completely in favor of native `print` function

### Optimizations

- [ ] Make a `BUILTIN_METHOD_RUNTIME_ERROR(class_name, method_name)` macro, which throws a runtime error with a nice prefix and always returns `NIL_VAL`. Use this in all `BUILTIN_METHOD_IMPL` functions.
- [ ] Remove `prop_getter`, `prop_setter`, `index_getter`, `index_setter` in favor of macros. E.g. `OBJ_GETTER()`, `SEQ_GETTER` etc. Currently, we always resolve the type with `typeof()` in `OP_GET_*` and `OP_SET_*`, but we have to handle all cases for the internal types anyway, so we can just `switch` on the values type and call the appropriate macro. If we ever decide to make these thing overridable by managed code, we'd ceratinly have to change this. Or maybe just inline them in the `OP_GET_*` and `OP_SET_*` functions, as it was before - cuz it's faster.
- [ ] Maybe add a fast hashtable-set function (key must be `ObjString`).
- [ ] Move `ip` into a register. This is a must-have. (**_See Challenge 24.1_**)
- [ ] Store strings as flexible array members (**_See Challenge 19.1_**)
- [ ] Constant time global variable lookup. (**_See Challenge 21.2_**)
- [ ] Cache function / method calls in the Vm
- [ ] Only necessary closures. Evaluate this, maybe it's not worth it. (**_See Challenge 25.1_**)
- [ ] Single-op unaries. Not fully-fledged constant folding, but a good start. (**_See Challenge 15.4_**)
- [ ] (Unsure) Differentiate between fields and methods textually. Use an array for lookup. Low prio. Would be nice to allow field-inheritance - this could be a major rewrite though. (**_See Challenge 28.3_**)

## Ideas

- [ ] Implement a register-based Vm https://www.lua.org/doc/jucs05.pdf
- [ ] Constant folding directly in the compiler
- [ ] Implement a JIT compiler
- [ ] Infer names of anonymous functions (Improves stack traces)

## References

- Based on the book [Crafting Interpreters](https://craftinginterpreters.com/). In contrast to the book, this Vm is not focused on space-efficiency, but more on ease of implementation and speed.
- https://github.com/kuroko-lang/kuroko
- https://github.com/Janko-dev/yabil/
- https://luajit.org/luajit.html

```

```
