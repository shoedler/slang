<img src="./doc/slang-dark.png" width="200">

# slang

A stack-based bytecode Vm written in C. It's a dynamically typed (_currently_), garbage-collected and object-oriented programming language with a syntax that is inspired by C#, JavaScript and Python.

## Roadmap

### Syntax & Language Features

- [ ] String interpolation. C#-style `$"Hello {name}"` (**_See Challenge 16.1_**)
- [ ] Add `const` (**_See Challenge 22.3_**)
- [ ] Implement `static fn foo -> nil` syntax for static methods. They are just another hashtable on the class object. Only accessible trough dot notation.
- [ ] Implement `in` operator. E.g. `item in Seq` (Check if item is in Seq), `item in Set` (Check if item is in Set), `key in Obj` (Check if key is in Obj), `string in Str` (Check if char / substr is in Str)
- [ ] Remove `Seq.has()` in favor of the `in` operator.
- [ ] Implement `Test` class / module with `Assert.that(expected, Is.equal_to(actual))`
- [ ] Implement `Set` class
- [ ] Implement `Set.add(Obj) -> Nil` as a builtin
- [ ] Implement `Set.del(Obj) -> Nil` as a builtin
- [ ] Implement `Set.has(Obj) -> Bool` as a builtin
- [ ] Implement `Set.len -> Num` as a builtin
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
- [ ] Implement Obj destructuring `let { a } = { a: 1 } // a == 1`
- [ ] Implement Seq destructuring `let [ a ] = [ 1 ] // a == 1`
- [ ] Implement `match` Statement. (**_See Challenge 23.1_**, on how to impl `switch`, that's a start.)
- [ ] Implement `nameof` keyword. E.g. `nameof(foo)` returns `"foo"`. (**_See Challenge 22.1_**)
- [ ] Implement `@memoize` decorator. How would this work? We would need be able to compare objects by their value instead of their reference (Stringification comes to mind - but that's slow). Maybe we can devise some kind of special hash function for this? E.g. for a seq, we could hash each element and then hash these hashes.
- [ ] Implement a way to add a doc string to functions and classes (managed code). This involves defining a new syntax for such a thing. Maybe attributes? (e.g. `@doc "This is a doc string"`)
- [ ] Implement iterators
- [ ] Implement `for ... in ...;` loops
- [ ] Rething notation for `for` and `while` loops. More like `each i in seq while i > 10` then you could do:
      <br>`each i in seq while i > 10 { }` -> essentially a normal `for` loop.
      <br>`each i in seq { }` -> essentially a normal `foreach` loop.
      <br>`while i > 10 { }` -> essentially a normal `while` loop.

## Improvements

- [ ] Closing over the loop variable. (**_See Challenge 25.2_**)
- [ ] Currently, `i++` behaves more like `++i` (Which we don't support). Fix it.
- [ ] Rename `typeof` to `typeof`
- [ ] Make `len` a property instead of a function?
- [ ] Add `error` to the reserved words
- [ ] Make sure managed code classes do not override internal classes.
- [ ] Move exit codes to `common.h` and replace all magic numbers with them
- [ ] Add a guard in `compiler.c -> number()` to check for overflow.
- [ ] Align error messages. Some use `'` around names, or type names, some don't.
- [ ] Add tests for `to_str`
- [ ] Allow return stats without suffixed `;`
- [ ] Remove `OP_PRINT` completely in favor of native `print` function

### Optimizations

- [ ] Maybe add a fast hashtable-set function (key must be `ObjString`).
- [ ] Add methods which are used frequently as props on `ObjClass` e.g. `method_to_str` etc.
- [ ] Add `get` and `set` methods to `ObjClass` to reduce the number of lookups.
- [ ] Add more words to `vm.cached_words` to reduce the number of lookups and string allocations.
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
