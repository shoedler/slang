<img src="./doc/slang-dark.png" width="200">

# slang

A stack-based bytecode Vm written in C. It's a dynamically typed (_currently_), garbage-collected and object-oriented programming language with a syntax that is inspired by C#, JavaScript and Python.

## Roadmap

### Syntax & Language Features

- [ ] `||` and `&&` operators.
- [ ] Ternaries. We can already achieve this with `and` + `or`, but it's not as nice. Syntax: `a ? b : c`, or `when a then b else c`. This is a must-have. (**_See Challenge 17.3_**)
- [ ] String interpolation. C#-style `$"Hello {name}"` (**_See Challenge 16.1_**)
- [ ] `continue` Statement (**_See Challenge 23.2_**)
- [ ] Closing over the loop variable. (**_See Challenge 25.2_**)
- [ ] Add `const` (**_See Challenge 22.3_**)
- [ ] `switch` Statement. Starting point for the `match` statement. (**_See Challenge 23.1_**)
- [ ] Implement `break`
- [ ] Implement `Test` class
- [ ] Implement `Seq.sort(sortFn) -> Seq` as a builtin
- [ ] Implement `Seq.slice(Num?, Num?) -> Seq` as a builtin
- [ ] Implement `Seq[1..] -> Seq` as a builtin (slice)
- [ ] Implement `Seq[..-1] -> Seq` as a builtin (slice)
- [ ] Implement `Seq[1..-1] -> Seq` as a builtin (slice)
- [ ] Implement `Seq[-1] -> Obj` as a builtin
- [ ] Implement `Gc` class
- [ ] Implement `Gc.collect() -> Nil` as a builtin
- [ ] Implement `Gc.stats() -> Obj` as a builtin
- [ ] Implement Map destructuring `let { a } = { a: 1 } // a == 1`
- [ ] Implement Seq destructuring `let [ a ] = [ 1 ] // a == 1`
- [ ] Implement `@memoize` decorator. How would this work? We would need be able to compare objects by their value instead of their reference (Stringification comes to mind - but that's slow). Maybe we can devise some kind of special hash function for this? E.g. for a seq, we could hash each element and then hash these hashes.
- [ ] Implement a way to add a doc string to functions and classes (managed code). This involves defining a new syntax for such a thing. Maybe attributes? (e.g. `@doc "This is a doc string"`)
- [ ] Implement iterators

## Improvements

- [ ] Add `error` to the reserved words
- [ ] Make sure managed code classes do not override internal classes.
- [ ] Move exit codes to `common.h` and replace all magic numbers with them
- [ ] Add a guard in `compiler.c -> number()` to check for overflow.
- [ ] Align error messages. Some use `'` around names, or type names, some don't.
- [ ] Add tests for to_str and overriden to_strs
- [ ] Allow return stats without suffixed `;`
- [ ] Remove `OP_PRINT` completely in favor of native `print` function

### Optimizations

- [ ] Implement a fast hashtable-get function which uses a shortened version of `find_entry`.
- [ ] Move `ip` into a register. This is a must-have. (**_See Challenge 24.1_**)
- [ ] Store strings as flexible array members (**_See Challenge 19.1_**)
- [ ] Improve `hash_value` and `values_equal`. I guess with the switch to allowing all values as keys it went down the drain.
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
