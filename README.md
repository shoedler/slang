<img src="./doc/logo/slang-dark.png" width="200">

# slang

A stack-based bytecode Vm written in C. It's a dynamically typed (_currently_), garbage-collected and object-oriented programming language with a syntax that is inspired by C#, JavaScript and Python.

## Roadmap for Version 1.0

### Syntax & Language Features

- [ ] Add tuples. E.g. `(1, 2, 3)`. They should be immutable and hash to their values.
  - Tests compiling tuples
  - Tests for tuple destructure
  - Tests for tuple equality
  - Tests for all tuple methods (listlike methods)
    - Make sure to compare the results (if the return tuples) with an identical tuple literal to ensure hashing is handled correctly within the methods.
  - Update existing tests involving all builtin types.
- [ ] Add nillish coalescing operator `??` e.g. `let x = [1] <newline> let v = x[1] ?? 0`
- [ ] String interpolation. C#-style `$"Hello {name}"` (**_See Challenge 16.1_**)
- [ ] Implement iterators. Maybe a new builtin class. They should initialize these fields/methods: `__has_next`, `__next()`.
- [ ] Implement `for ... in ...;` loops
- [ ] Add `const` (**_See Challenge 22.3_**)
- [ ] Add destructuring to module imports.
- [ ] Implement `Json` module
- [ ] Implement `Test` class / module with `Assert.that(expected, Is.equal_to(actual))`
- [ ] Implement `Set` class
- [ ] Implement `Set.add(Obj) -> Nil` as a builtin
- [ ] Implement `Set.del(Obj) -> Nil` as a builtin
- [ ] Implement `Seq(Set)` constructor
- [ ] Implement `Seq.sort(sortFn) -> Seq` as a builtin
- [ ] Implement `Gc` module
- [ ] Implement `Gc collect() -> Nil` as a builtin
- [ ] Implement `Gc stats() -> Obj` as a builtin
- [ ] Implement `match` Statement. (**_See Challenge 23.1_**, on how to impl `switch`, that's a start.)
- [ ] Implement `nameof` keyword. E.g. `nameof(foo)` returns `"foo"`. (**_See Challenge 22.1_**)
- [ ] Implement `@memoize` decorator. How would this work? We would need be able to compare objects by their value instead of their reference (Stringification comes to mind - but that's slow). Maybe we can devise some kind of special hash function for this? E.g. for a seq, we could hash each element and then hash these hashes.
- [ ] Implement a way to add a doc string to functions and classes (managed code). This involves defining a new syntax for such a thing. Maybe attributes? (e.g. `@doc "This is a doc string"`)

## Improvements

- [ ] Add tests for `OP_MODULO`
- [ ] (When `Gc` is implemented) Add some test cases where we call `Gc.collect()` to ensure that relevant objects are not collected.
- [ ] Check `can_assign` in `tuple_literal`, `seq_literal` and `obj_literal`. It should be false. Or implement destructuring assignments.
- [ ] Call `to_str` implicitly when adding a string to a non-string.
- [ ] Remove in `VALUE_STR_CLASS` (`<Class X>`) the "Class" prefix from class type names.
- [ ] Refactor `VALUE_STR_NATIVE` to look more like the normal `<Fn "name">` strings.
- [ ] Use something else instead of `rint`, because it's not very precise. See _num-to-str.spec.sl_ for an example.
- [ ] Closing over the loop variable. (**_See Challenge 25.2_**)
- [ ] Currently, `i++` behaves more like `++i` (Which we don't support). Fix it.
- [ ] Add `error` to the reserved words
- [ ] Make sure managed code classes do not override internal classes.
- [ ] Move exit codess to `common.h` and replace all magic numbers with them
- [ ] Add a guard in `compiler.c -> number()` to check for overflow.
- [ ] Align error messages. Some use `'` around names, or type names, some don't.
- [ ] Allow return stats without suffixed `;`
- [ ] Remove `OP_PRINT` completely in favor of native `print` function

### Optimizations

- [ ] Make stringification faster.
- [ ] Inline `push()`, `peek()` and `pop()` in the Vm.
- [ ] Make a `BUILTIN_METHOD_RUNTIME_ERROR(class_name, method_name)` macro, which throws a runtime error with a nice prefix and always returns `NIL_VAL`. Use this in all `BUILTIN_METHOD_IMPL` functions.
- [ ] Maybe add a fast hashtable-set function (key must be `ObjString`).
- [ ] Move `ip` into a register. This is a must-have. (**_See Challenge 24.1_**)
- [ ] Store strings as flexible array members (**_See Challenge 19.1_**)
- [ ] Constant time global variable lookup. (**_See Challenge 21.2_**)
- [ ] Only necessary closures. Evaluate this, maybe it's not worth it. (**_See Challenge 25.1_**)
- [ ] Single-op unaries. Not fully-fledged constant folding, but a good start. (**_See Challenge 15.4_**)

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
