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

- [ ] Implement `for ... in ...;` loops (Implement Iterators)
- [ ] Add nillish coalescing operator `??` e.g. `let x = [1] <newline> let v = x[1] ?? 0`
- [ ] Implement native `Json` module.
  - [ ] Implement `Json.parse(Str) -> Obj`.
  - [ ] Implement `Json.stringify(Value) -> Str`.
  - [ ] Implement `Json.stringify(Value, Int) -> Str`. (Indentation)
- [ ] Implement `Seq.cull(Value|Fn) -> Seq`. (Remove all elements that are equal to the argument or satisfy the predicate)
- [ ] Implement `Seq.zip(Seq, Seq) -> Seq`. (Zip two sequences into one sequence of tuples)
- [ ] Add a variant of `log` (Maybe `tap`/`info`/`dump`/`peek`?) which accepts a single argument and also, return it. That'd be awesome: `const x = a + b + c + tap(d) + e`
- [ ] Add more error classes to std. Add a native `Error` base class, from which managed-code errors inherit. Check `vm_inherits(error.type, vm.error_class)` in `handle_runtime_error` and - if true - use `error.type->name` as the prefix instead of `Uncaught error`.
- [x] ~~Implement `Seq.sum() -> Num`, `Tuple.sum() -> Num`. (Sum all elements. Requires some sort of __add)~~
- [x] ~~Implement `Seq.sort(sort_fn) -> Seq`. (Sort a sequence in place. Requires some sort of __lt)~~
- [x] ~~Implement native `Math` module.~~
  - [x] ~~Implement `Math.abs(Num) -> Num`.~~
  - [x] ~~Implement `Math.ceil(Num) -> Int`.~~
  - [x] ~~Implement `Math.floor(Num) -> Int`.~~
- [x] ~~Implement `Test` class / module with `Assert.that(expected, Is.equal_to(actual))`~~
- [x] ~~Add destructuring to module imports.~~
- [x] ~~Add `const` (**_See Challenge 22.3_**)~~
- [x] ~~Implement `Set` class~~ (Part of the `std` module - not a native type)
- [x] ~~Implement `Set.add(Obj) -> Nil`.~~
- [x] ~~Implement `Set.del(Obj) -> Nil`.~~
- [x] ~~Implement ~~`Seq(Set)` constructor~~ `Set.to_seq() -> Seq`~~
- [x] ~~Implement `Gc` module~~
- [x] ~~Implement `Gc collect() -> Nil`.~~
- [x] ~~Implement `Gc stats() -> Obj`.~~

## Improvements

- [ ] Improve destructuring assignment:
  - [ ] Check `can_assign` in `tuple_literal`, `seq_literal` and `obj_literal`. It should be false. Or implement destructuring assignments.
  - [ ] If you destructure a `Seq` into a `Tuple`, the rest of the elements should be of the type of the lhs. E.g. `let (a, ...b) = [1, 2, 3]` where `a` is an `Int` and `b` is a `Tuple`. Currently, `b` is a `Seq`.
- [ ] Make managed-code callables accept less arguments than it has parameters. There is an inconsistency, since native-callables allow this. Should be easy, just pass `nil` to the missing arguments.
- [ ] Currently, `i++` behaves more like `++i` (Which we don't support). Fix it.
- [ ] Add a guard in `compiler.c -> number()` to check for overflow.
- [ ] Remove `OP_PRINT` completely in favor of native `log` function
- [ ] Add a mimalloc segfault handler.
- [ ] Maybe check for `NULL` functions in `vm_exec_callable` instead of before calling it - would add some overhead though.
- [x] ~~Add `Tuple.order` test.~~
- [x] ~~Remove `"" + value.to_str()` throughout the codebase, `"" + value` should now work.~~
- [x] ~~Align `DO_OP_IN` with `MAKE_OP()`, there's some unnecessary push/pop-int in there.~~
- [x] ~~Call `to_str` implicitly when adding a string to a non-string. Only if the left side is a string.~~
- [x] ~~Fix VM finishing a program in error state not exiting with `EXIT_FAILURE`. (Probably, the flags are reset in `vm_free` or `reset_stack` or something).~~
- [x] ~~Make compiler errors recoverable. Just let it sync, then continue - instead of aborting.~~
- [x] ~~Use `obj_get_prop` in `obj_has` instead of just checking the hashtable for a value. Otherwise, they behave differently - which sucks.~~
- [x] ~~Make sure managed code classes do not override internal classes.~~ Not necessary, since we don't update the Vms internalized natives. 
- [x] ~~Add `error` and other contextual keywords to a list of reserved words (Maybe including all natives). Check them when declaring anything.~~ Not necessary, since other ctx keywords have their own TOKEN type. As for errors, we'll just allow redeclaration, even inside a catch-block. The reason being that the error-var gets injected into a scope surrounding the whole try/catch statement, not just in the catch-block.
- [x] ~~Remove `string_to_double` and use `number` from the compiler instead.~~
- [x] ~~Generalized calls. This is optional, but could enhance the language.~~
  - [x] ~~Move `hash_value` to types (`uint64_t ObjClass.__hash(value: Value)`)~~
  - [x] ~~Move `values_equal` to types (`bool ObjClass.__equals(a: Value, b: Value)`) **DUPE**, see "Optimizations".~~
  - [x] ~~Move `to_str` to types (`Obj ObjClass.__to_str(value: Value)`)~~
- [x] ~~Remove the "Class" prefix in `VALUE_STR_CLASS` to just return the class name. This is a bit more consistent with the rest of the code.~~
- [x] ~~Restructure test: compiler, vm (types, modules)~~
- [x] ~~(When `Gc` is implemented) Add some test cases where we call `Gc.collect()` to ensure that relevant objects are not collected.~~
- [x] ~~Add tests for `Fn.bind(Obj)`~~
- [x] ~~Add Tests with tabs in source code. Especially to test uncaught runtime error reporting.~~
- [x] ~~Add tests for `OP_MODULO`~~
- [x] ~~Module Caching refactor: When trying to load a module from cache, we need to check for the absolute file path of the module instead of only the name. When importing a module with destructuring, we use the absolute path as the module name - when you load the same module with a relative path and a module name, it will currently be cached with the key "module name" and therefore not be found in the cache.~~

## Codebase Refactors

- [ ] Align doc-comment style to use `/**` everywhere.
- [ ] Align error messages. Some use `'` around names, or type names, some don't.
- [ ] Make a `NATIVE_METHOD_RUNTIME_ERROR(class_name, method_name)` macro, which throws a runtime error with a nice prefix and always returns `NIL_VAL`. Use this in all `NATIVE_METHOD_IMPL` functions.
- [ ] Move utility functions from vm.c to respective files.

## Optimizations

- [ ] Add `OP_CALL_SP` and use a lookup table similar to `finalize_new_class` to call the method. Would make sense now that we have so many SP methods. We could also remove all arithmetic and comparison operations.
- [ ] The features I want to implement for this language kinda require a more complex compilation process. Specifically, ditching the single-pass approach for a two-pass compiler and an actual AST.
- [ ] Maybe delete `NATIVE_CHECK_RECEIVER` as most of the time the method is called from the receiver.
- [ ] Use `memcpy` for concat and such (See `Seq(Tuple)` ctor for an example). Check for for-loops in the native methods. Need to test if this copies values by reference or by value. Needs a decision on on how we want to handle that.
- [ ] Implement a string builder and use it everywhere. This is a must-have.
- [ ] Inline `vm_push()`, `peek()` and `vm_pop()` in the Vm.
- [ ] Maybe add a fast hashtable-set function (key must be `ObjString`).
- [ ] Move `ip` into a register. This is a must-have. (**_See Challenge 24.1_**)
- [ ] Store strings as flexible array members (**_See Challenge 19.1_**)
- [ ] Split globals into a `Hastable global_names` and a `ValueArray global_values`. This would allow for constant time global variable lookup. (**_See Challenge 21.2_**) https://github.com/munificent/craftinginterpreters/blob/master/note/answers/chapter21_global.md
- [ ] Only necessary closures. Evaluate this, maybe it's not worth it. (**_See Challenge 25.1_**)
- [ ] Closing over the loop variable. (**_See Challenge 25.2_**)
- [ ] Single-op unaries. Not fully-fledged constant folding, but a good start. (**_See Challenge 15.4_**)
- [x] ~~Move `values_equal` to types (`bool ObjClass.__equals(a: Value, b: Value)`) - this would make `values_equal` obsolete.~~

---

# Roadmap for Version 2.0

## Features

- [ ] String interpolation. C#-style `$"Hello {name}"` (**_See Challenge 16.1_**)
- [ ] Implement `match` Statement. (**_See Challenge 23.1_**, on how to impl `switch`, that's a start.)
- [ ] Implement `@memoize` decorator. Would put args into a `Tuple` and use that as a key in a `Obj`.
- [ ] Implement `Float.nan` and `Float.inf` constants (Would require static fields).

---

# Roadmap for Version 3.0

## Features

### Type-checking

- [ ] Implement syntax for type annotations. E.g. `let x: Int = 1`. We'll check as much as possible at compile-time. Locals are already cared for, because they live on the stack and are not referenced by a name. All locals are resolved during compile time. The only exception being locals that are not initialized with a value. That should be allowed, but the type must be declared. E.g. `let x: Int`. This is a bit more flexible than C# and a bit less flexible than TypeScript. We'll see how it goes.
- [ ] Implement a solution for functions. We should introduce a signature field in fn objects. The sig struct should probably consist of: `args: ObjClass**` to store the types of the arguments, `ret: ObjClass*` to store the return type of the function, `argc: int` to store the number of arguments.
- [ ] Introduce type-modifiers. `?` for nullable, and maybe something for exact type match. This should be done as a flag on the `Value` struct. Maybe make a `Type` struct that holds the `ObjClass*` and the flags. This could then also be used for function signatures.
- [ ] Actually add typechecking and remove all `NATIVE_CHECK_ARG` things.

### LSP Server

- [ ] Implement an LSP server. Rough idea: Compile the source and generate the bytecode. Maybe with some heuristics. Like, don't recompile imported modules. We should be able to map a line + column position to a instruction, since we store the relevant information in a Token for each instruction. We should also be able to retrieve a list of possible strings to write next in the sourcecode - e.g. globals, methods etc.

---

## Further Ideas

- Rebuild to a register-based Vm https://www.lua.org/doc/jucs05.pdf
- Constant folding directly in the compiler
- Implement a JIT compiler
