<img src="./doc/logo/slang-dark.png" width="200">

# slang

A stack-based bytecode Vm written in C. It's a dynamically typed (_currently_), garbage-collected and object-oriented programming language with a syntax that is inspired by C#, JavaScript and Python.

## Roadmap for Version 1.0

### Syntax & Language Features

- [ ] Implement `for ... in ...;` loops
- [ ] Add nillish coalescing operator `??` e.g. `let x = [1] <newline> let v = x[1] ?? 0`
- [ ] String interpolation. C#-style `$"Hello {name}"` (**_See Challenge 16.1_**)
- [ ] Add `const` (**_See Challenge 22.3_**)
- [ ] Add destructuring to module imports.
- [ ] Implement `Json` module
- [ ] Implement `Math` module
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
- [ ] Implement `Float.nan` and `Float.inf` constants.
- [ ] Implement `@doc "bla bla"` decorator to add a doc string to functions and classes (managed code). This involves defining a new syntax for such a thing.
- [ ] Implement `@memoize` decorator. How would this work? We would need be able to compare objects by their value instead of their reference (Stringification comes to mind - but that's slow). Maybe we can devise some kind of special hash function for this? E.g. for a seq, we could hash each element and then hash these hashes.

## Improvements

- [ ] Add Tests with tabs in source code. Especially to test uncaught runtime error reporting.
- [ ] Add tests for `OP_MODULO`
- [ ] Add tests for `Fn.bind(Obj)`
- [ ] Restructure test: compiler, vm (types, modules)
- [ ] (When `Gc` is implemented) Add some test cases where we call `Gc.collect()` to ensure that relevant objects are not collected.
- [ ] Improve destructuring assignment:
  - [ ] Check `can_assign` in `tuple_literal`, `seq_literal` and `obj_literal`. It should be false. Or implement destructuring assignments.
  - [ ] If you destructure a `Seq` into a `Tuple`, the rest of the elements should be of the type of the lhs. E.g. `let (a, ...b) = [1, 2, 3]` where `a` is an `Int` and `b` is a `Tuple`. Currently, `b` is a `Seq`.
- [ ] Call `to_str` implicitly when adding a string to a non-string. Only if the left side is a string.
- [ ] Remove the "Class" prefix in `VALUE_STR_CLASS` to just return the class name. This is a bit more consistent with the rest of the code.
- [ ] Use something else instead of `rint`, because it's not very precise. See _num-to-str.spec.sl_ for an example.
- [ ] Closing over the loop variable. (**_See Challenge 25.2_**)
- [ ] Currently, `i++` behaves more like `++i` (Which we don't support). Fix it.
- [ ] Make sure managed code classes do not override internal classes.
- [ ] Add a guard in `compiler.c -> number()` to check for overflow.
- [ ] Remove `OP_PRINT` completely in favor of native `print` function
- [ ] Add `error` to the reserved words
- [ ] Align error messages. Some use `'` around names, or type names, some don't.

### Optimizations

- [ ] Use `memcpy` for concat and such (See `Seq(Tuple)` ctor for an example). Check for for-loops in the builtin methods.
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
- [ ] Implement an LSP server. Rough idea: Compile the source and generate the bytecode. Maybe with some heuristics. Like, don't recompile imported modules. We should be able to map a line + column position to a instruction, since we store the relevant information in a Token for each instruction. We should also be able to retrieve a list of possible strings to write next in the sourcecode - e.g. globals, methods etc.
- [ ] Constant folding directly in the compiler
- [ ] Implement a JIT compiler

## References

- Based on the book [Crafting Interpreters](https://craftinginterpreters.com/). In contrast to the book, this Vm is not focused on space-efficiency, but more on ease of implementation and speed.
- https://github.com/kuroko-lang/kuroko
- https://github.com/Janko-dev/yabil/
- https://luajit.org/luajit.html
