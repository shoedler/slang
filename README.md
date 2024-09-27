<img src="./doc/logo/slang-dark.png" width="200">

# slang

A stack-based bytecode Vm written in C. It's a dynamically typed (_currently_), garbage-collected and object-oriented programming language with a syntax that is inspired by C#, JavaScript and Python - just less verbose.

> [!NOTE]
>
> Slang is still in development. The language is not stable and the syntax is subject to change.

## Why?

The main goal of this language is to be able to solve problems with succinct code. It's aimed at developers who like competitive programming or just want to write code that is concise. It's also a great language to learn how programming languages work under the hood, because it's relatively simple.

It's a less-verbose language that relies on the JavaScript syntax, but also incorporates some of the features of C# and Python.
It also tries to approach some things differently than JavaScript - The major "thing" being the fact that you can use reference types to index into objects and the presence of a tuple type, which features a reproducible hash. This is particularly nice if you're into dynamic-programming.
You can, for example, easily cache stuff:

![image](https://github.com/user-attachments/assets/ab4f1c28-52e6-4bd0-b519-657355f806fa)

> [!TIP]
> Currently, there is no Wiki or "Getting Started" or the like.
> The best way to see what it can do is to checkout `sample.sl` in the repository root, or the tests in the `test` directory.

## References

- Based on the book [Crafting Interpreters](https://craftinginterpreters.com/). In contrast to the book, this Vm is not focused on space-efficiency, but more on ease of implementation and speed.
- https://github.com/kuroko-lang/kuroko
- https://github.com/Janko-dev/yabil/
- https://luajit.org/luajit.html

---

# Roadmap for Version 1.0

## Features

### Type-checking

- [ ] Implement syntax for type annotations. E.g. `let x: Int = 1`. We'll check as much as possible at compile-time. Locals are already cared for, because they live on the stack and are not referenced by a name. All locals are resolved during compile time. The only exception being locals that are not initialized with a value. That should be allowed, but the type must be declared. E.g. `let x: Int`. This is a bit more flexible than C# and a bit less flexible than TypeScript. We'll see how it goes.
- [ ] Implement a solution for functions. We should introduce a signature field in fn objects. The sig struct should probably consist of: `args: ObjClass**` to store the types of the arguments, `ret: ObjClass*` to store the return type of the function, `argc: int` to store the number of arguments.
- [ ] Introduce type-modifiers. `?` for nullable, and maybe something for exact type match. This should be done as a flag on the `Value` struct. Maybe make a `Type` struct that holds the `ObjClass*` and the flags. This could then also be used for function signatures.
- [ ] Actually add typechecking and remove all `NATIVE_CHECK_ARG` things.

### Language Features

- [ ] Implement `Int` division with `//`.
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
- [x] ~~Implement `Gc` module~~
- [x] ~~Implement `Gc collect() -> Nil` as a builtin~~
- [x] ~~Implement `Gc stats() -> Obj` as a builtin~~
- [ ] Implement `match` Statement. (**_See Challenge 23.1_**, on how to impl `switch`, that's a start.)
- [ ] Implement `nameof` keyword. E.g. `nameof(foo)` returns `"foo"`. (**_See Challenge 22.1_**)
- [ ] Implement `@memoize` decorator. Would put args into a `Tuple` and use that as a key in a `Obj`.
- [ ] Implement `Float.nan` and `Float.inf` constants (Would require static fields).

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
- [ ] Closing over the loop variable. (**_See Challenge 25.2_**)
- [ ] Currently, `i++` behaves more like `++i` (Which we don't support). Fix it.
- [ ] Make sure managed code classes do not override internal classes.
- [ ] Add a guard in `compiler.c -> number()` to check for overflow.
- [ ] Remove `OP_PRINT` completely in favor of native `print` function
- [ ] Add `error` to the reserved words
- [ ] Align error messages. Some use `'` around names, or type names, some don't.
- [ ] Generalized calls. This is optional, but could enhance the language.
  - [ ] Move `hash_value` to types (`uint64_t ObjClass.__hash(value: Value)`)
  - [ ] Move `to_str` to types (`Obj ObjClass.__to_str(value: Value)`)
  - [ ] Move `values_equal` to types (`bool ObjClass.__eq(a: Value, b: Value)`) **DUPE**, see "Optimizations".

## Optimizations

- [ ] Move `values_equal` to types (`bool ObjClass.__eq(a: Value, b: Value)`) - this would make `values_equal` obsolete.
- [ ] Make a `immortal` flag on `Obj` to prevent it from being collected. This could be a big perf gain - though we still have to make sure that nested objects "inherit" this flag.
- [ ] Use `memcpy` for concat and such (See `Seq(Tuple)` ctor for an example). Check for for-loops in the builtin methods. Need to test if this copies values by reference or by value. Needs a decision on on how we want to handle that.
- [ ] Make stringification faster.
- [ ] Inline `push()`, `peek()` and `pop()` in the Vm.
- [ ] Make a `NATIVE_METHOD_RUNTIME_ERROR(class_name, method_name)` macro, which throws a runtime error with a nice prefix and always returns `NIL_VAL`. Use this in all `NATIVE_METHOD_IMPL` functions.
- [ ] Maybe add a fast hashtable-set function (key must be `ObjString`).
- [ ] Move `ip` into a register. This is a must-have. (**_See Challenge 24.1_**)
- [ ] Store strings as flexible array members (**_See Challenge 19.1_**)
- [ ] Constant time global variable lookup. (**_See Challenge 21.2_**)
- [ ] Only necessary closures. Evaluate this, maybe it's not worth it. (**_See Challenge 25.1_**)
- [ ] Single-op unaries. Not fully-fledged constant folding, but a good start. (**_See Challenge 15.4_**)

## Further Ideas

> Not part of the 1.0 release, but still interesting.

- [ ] Implement a register-based Vm https://www.lua.org/doc/jucs05.pdf
- [ ] Implement an LSP server. Rough idea: Compile the source and generate the bytecode. Maybe with some heuristics. Like, don't recompile imported modules. We should be able to map a line + column position to a instruction, since we store the relevant information in a Token for each instruction. We should also be able to retrieve a list of possible strings to write next in the sourcecode - e.g. globals, methods etc.
- [ ] Constant folding directly in the compiler
- [ ] Implement a JIT compiler
