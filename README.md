<img src="./doc/slang-dark.png" width="200">

# slang

A stack-based bytecode Vm written in C, based on the book [Crafting Interpreters](https://craftinginterpreters.com/).
In contrast to the book, this Vm is not focused on space-efficiency, but more on ease of implementation and speed.

## Roadmap

### Initial

- [x] Base implementation of the VM
- [x] Fix GC Bug in current sample script
- [x] Align fn syntax
- [x] Add tests according to the original repository of bob nystrom
- [x] `cls B < A {}` crashes the VM
- [x] Re-read the book & comment the code accordingly (Continue in chapter 27)
- [x] Merge "function" and "functions" tests directories
- [x] Rename "super" test cases to "base"

### Syntax & Language Features

- [ ] Add tests for anonymous functions
- [ ] Implement `+=`, `-=`, `*=`, `/=`, `%=`
- [ ] Challenges to implement:
  - [ ] 24 $\to$ 1: Move `ip` into a register. This is a must-have.
  - [x] 14 $\to$ 2: Allow more than 256 constants per chunk ~~by adding `OP_CONSTANT_LONG`~~. This is a must-have. (Just bumped the stack-width to 16 bits - lol)
  - [x] 22 $\to$ 4: Allow more than 256 locals per scope. This is a must-have. (Just bumped the stack-width to 16 bits - lol)
  - [ ] 17 $\to$ 3: Ternaries. We can already achieve this with `and` + `or`, but it's not as nice. Syntax: `a ? b : c`, or `a then b else c`. This is a must-have.
  - [ ] 16 $\to$ 1: String interpolation. C#-style `$"Hello {name}"`
  - [ ] 21 $\to$ 2: Constant time global variable lookup.
  - [ ] 23 $\to$ 2: `continue` Statement
  - [ ] 23 $\to$ 1: `switch` Statement. Starting point for the `match` statement.
  - [x] 20 $\to$ 1: Index a hashtable with any primitive. ~~This is a good introduction to the goal of being able to index with reference types.~~ $\to$ Done. Added indexing by reference aswell.
  - [ ] 19 $\to$ 1: Store strings as flexible array members
  - [ ] 25 $\to$ 2: Closing over the loop variable.
  - [ ] 22 $\to$ 3: Add `const`
  - [ ] 15 $\to$ 4: Single-op unaries. Not fully-fledged constant folding, but a good start.
  - [ ] 25 $\to$ 1: Only necessary closures. Evaluate this, maybe it's not worth it.
  - [ ] 28 $\to$ 1: Cache `ctor` keyword. Low prio.
  - [ ] 28 $\to$ 3: Differentiate between fields and methods textually. Use an array for lookup. Low prio. Would be nice to allow field-inheritance - this could be a major rewrite though.
- [ ] Implement `break`
- [x] Implement sequences
- [ ] Implement hashtable keys for all types: Objs are by reference, primitives and strings by value
- [ ] Allow return stats without suffixed `;`

## Improvements

- [ ] Move exit codes to `common.h` and replace all magic numbers with them
- [ ] Align error messages. Some use `'` around names, or type names, some don't.
- [ ] Make macros in `value.h` for internal types and their values (e.g. `true` and `false`).
- [ ] Add a test for `ObjBoundMethod` where it's method is a `NativeFn`, because it was previously only a `ObjClosure`.

### Optimizations

- [ ] ⚠️ Re-add `OP_INVOKE`
- [ ] Improve `hash_value` and `values_equal`. I guess with the switch to allowing all values as keys it went down the drain.
- [ ] Implement a fast hashtable-get function which uses a shortened version of `find_entry`.
- [ ] Cache modules in the Vm. It's awfully slow and happens during runtime... I hate that very much.
- [ ] Cache function / method calls in the Vm

### Modularity & Standard Library

- The active call frame is the source of the current "globals".
- Every closure object has a reference to its globals.
- When a closure is created for a function, it should capture the current globals and use them as its own globals. This is necessary to implement modules.
- Then, to import a module.
- There probably needs to be a reference to the owner of the closures globals, so that the gc collects it when the module is no longer used.

Devise a system to load modules. I want most of the stdlib to be native code to improve performance, but I also want to be able to load modules from disk.

- [ ] Remove `OP_PRINT` completely in favor of native `print` function
- [ ] Add type conversion functions. Proposal: `int()`, `float()`, `to_str()`, `bool()`
- [ ] Add type checking functions. Proposal: `type()` and `Num`, `Str`, `Bool` or `@num`, `@str`, `@bool`
- [ ] Add sequences
- [ ] Add objects

## Ideas

- [ ] Implement a register-based Vm https://www.lua.org/doc/jucs05.pdf
- [ ] Constant folding directly in the compiler
- [ ] Implement a JIT compiler
- [ ] Infer names of anonymous functions (Improves stack traces)

## References

- https://github.com/Janko-dev/yabil/
- https://luajit.org/luajit.html
