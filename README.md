<img src="./doc/slang-dark.png" width="200">

# slang

A stack-based bytecode Vm written in C. It's a dynamically typed (_currently_), garbage-collected and object-oriented programming language with a syntax that is inspired by C#, JavaScript and Python.

## Roadmap

### Syntax & Language Features

- [ ] Implement `+=`, `-=`, `*=`, `/=`, `%=`
- [ ] Ternaries. We can already achieve this with `and` + `or`, but it's not as nice. Syntax: `a ? b : c`, or `a then b else c`. This is a must-have. (**_See Challenge 17.3_**)
- [ ] String interpolation. C#-style `$"Hello {name}"` (**_See Challenge 16.1_**)
- [ ] `continue` Statement (**_See Challenge 23.2_**)
- [ ] Store strings as flexible array members (**_See Challenge 19.1_**)
- [ ] Closing over the loop variable. (**_See Challenge 25.2_**)
- [ ] Add `const` (**_See Challenge 22.3_**)
- [ ] `switch` Statement. Starting point for the `match` statement. (**_See Challenge 23.1_**)
- [ ] Implement `break`
- [ ] Implement iterators
- [ ] Implement a way to add a doc string to functions and classes.
- [ ] Implement a way to add a synopsis to functions and classes. Kinda like a interface. E.g. for a seq ctor `Seq(arg_a: Num) -> Seq // Of length arg_a`, `Seq(arg_a: Str) -> Seq // Containing each char of arg_a`. This should also work for native functions
- [ ] Implement a ctor for `Seq(Str)`
- [ ] Implement `Seq.sort()` as a builtin
- [ ] Implement `Seq.filter()` as a builtin
- [ ] Implement `Seq.map()` as a builtin
- [ ] Implement `Seq.reduce()` as a builtin
- [ ] Implement `Seq.find()` as a builtin
- [ ] Implement `Seq.contains()` as a builtin
- [ ] Implement `Seq.join()` as a builtin
- [ ] Implement `Seq.slice()` as a builtin
- [ ] Implement `Seq.reverse()` as a builtin
- [ ] Implement `Seq[1..]` as a builtin
- [ ] Implement `Seq[..-1]` as a builtin
- [ ] Implement `Seq[1..-1]` as a builtin
- [ ] Implement `Seq[-1]` as a builtin

## Improvements

- [ ] Move exit codes to `common.h` and replace all magic numbers with them
- [ ] Align error messages. Some use `'` around names, or type names, some don't.
- [ ] Add tests for to_str and overriden to_strs
- [ ] Allow return stats without suffixed `;`
- [ ] Remove `OP_PRINT` completely in favor of native `print` function

### Optimizations

- [ ] Implement a fast hashtable-get function which uses a shortened version of `find_entry`.
- [ ] Move `ip` into a register. This is a must-have. (**_See Challenge 24.1_**)
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
