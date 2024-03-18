<img src="./doc/slang-dark.png" width="200">

# slang

A stack-based bytecode Vm written in C. It's a dynamically typed (_currently_), garbage-collected and object-oriented programming language with a syntax that is inspired by C, JavaScript and Python.

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

## Improvements

- [ ] Move exit codes to `common.h` and replace all magic numbers with them
- [ ] Align error messages. Some use `'` around names, or type names, some don't.
- [ ] Add tests for to_str and overriden to_strs
- [ ] Allow return stats without suffixed `;`
- [ ] Remove `OP_PRINT` completely in favor of native `print` function
- [x] Add tests for anonymous functions
- [x] Add tests for sequences
- [x] Allow more than 256 constants per chunk ~~by adding `OP_CONSTANT_LONG`~~. This is a must-have. (Just bumped the stack-width to 16 bits - lol) (**_See Challenge 14.2_**)
- [x] Allow more than 256 locals per scope. This is a must-have. (Just bumped the stack-width to 16 bits - lol) (**_See Challenge 22.4_**)
- [x] Add tests for modules
- [x] Update tests for native functions
- [x] Add test for bound native methods.
- [x] Make macros in `value.h` for internal types and their values (e.g. `true` and `false`).
- [x] Add a test for `ObjBoundMethod` where it's method is a `NativeFn`, because it was previously only a `ObjClosure`.
- [x] Add type conversion functions. Proposal: `int()`, `float()`, `to_str()`, `bool()`
- [x] Add type checking functions. Proposal: `type()` and `Num`, `Str`, `Bool` or `@num`, `@str`, `@bool`
- [x] Add sequences
- [x] Index a hashtable with any primitive. ~~This is a good introduction to the goal of being able to index with reference types.~~ $\to$ Done. Added indexing by reference aswell. (**_See Challenge 20.1_**)

### Optimizations

- [ ] Implement a fast hashtable-get function which uses a shortened version of `find_entry`.
- [ ] Move `ip` into a register. This is a must-have. (**_See Challenge 24.1_**)
- [ ] Improve `hash_value` and `values_equal`. I guess with the switch to allowing all values as keys it went down the drain.
- [ ] Constant time global variable lookup. (**_See Challenge 21.2_**)
- [ ] Cache function / method calls in the Vm
- [ ] Only necessary closures. Evaluate this, maybe it's not worth it. (**_See Challenge 25.1_**)
- [ ] Single-op unaries. Not fully-fledged constant folding, but a good start. (**_See Challenge 15.4_**)
- [ ] (Unsure) Differentiate between fields and methods textually. Use an array for lookup. Low prio. Would be nice to allow field-inheritance - this could be a major rewrite though. (**_See Challenge 28.3_**)
- [x] ⚠️ Re-add `OP_INVOKE`
- [x] Cache modules in the Vm. It's awfully slow and happens during runtime... I hate that very much.
- [x] Cache `ctor` keyword. Low prio. (**_See Challenge 28.1_**)

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
