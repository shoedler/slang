<img src="./doc/slang-dark.png" width="200">

# slang

A stack-based bytecode VM written in C, based on the book [Crafting Interpreters](https://craftinginterpreters.com/).

## TODOs

### Phase 1

- [x] Base implementation of the VM
- [x] Fix GC Bug in current sample script
- [x] Align fn syntax
- [x] Add tests according to the original repository of bob nystrom
- [x] `cls B < A {}` crashes the VM

### Phase 2

- [ ] Re-read the book & comment the code accordingly (Continue in chapter 27)
- [ ] Merge "function" and "functions" tests directories
- [ ] Rename "super" test cases to "base"
- [ ] Add tests for anonymous functions
- [ ] Challenges to implement:
  - [ ] 24 $\to$ 1: Move `ip` into a register. This is a must-have.
  - [ ] 14 $\to$ 2: Allow more than 256 constants per chunk by adding `OP_CONSTANT_LONG`. This is a must-have.
  - [ ] 22 $\to$ 4: Allow more than 256 locals per scope. This is a must-have.
  - [ ] 17 $\to$ 3: Ternaries. We can already achieve this with `and` + `or`, but it's not as nice. Syntax: `a ? b : c`, or `a ? b else c`. This is a must-have.
  - [ ] 16 $\to$ 1: String interpolation. C#-style `$"Hello {name}"`
  - [ ] 21 $\to$ 2: Constant time global variable lookup.
  - [ ] 23 $\to$ 1: `switch` Statement. Starting point for the `match` statement.
  - [ ] 23 $\to$ 2: `continue` Statement
  - [ ] 20 $\to$ 1: Index a hashtable with any primitive. This is a good introduction to the goal of being able to index with reference types.
  - [ ] 19 $\to$ 1: Store strings as flexible array members
  - [ ] 25 $\to$ 2: Closing over the loop variable.
  - [ ] 22 $\to$ 3: Add `const`
  - [ ] 15 $\to$ 4: Single-op unaries. Not fully-fledged constant folding, but a good start.
  - [ ] 25 $\to$ 1: Only necessary closures. Evaluate this, maybe it's not worth it.
  - [ ] 28 $\to$ 1: Cache `ctor` keyword. Low prio.
  - [ ] 28 $\to$ 3: Differentiate between fields and methods textually. Use an array for lookup. Low prio.

### Phase 3

- [ ] Allow return stats without suffixed `;`
- [ ] Remove `OP_PRINT` completely in favor of native `print` function
- [ ] Add type conversion functions. Proposal: `int()`, `float()`, `str()`, `bool()`
- [ ] Add type checking functions. Proposal: `type()` and `Num`, `Str`, `Bool` or `@num`, `@str`, `@bool`
- [ ] Add sequences
- [ ] Add objects

### Phase 4

- [ ] Implement `break`
- [ ] Implement `inc` and `dec` operators, maybe as a separate OpCode
- [ ] Implement `+=`, `-=`, `*=`, `/=`, `%=`

### Phase 5

- [ ] Implement hashtable keys for all types: Objs are by reference, primitives and strings by value
- [x] Implement sequences

### Ideas

- [ ] Implement register-based VM https://www.lua.org/doc/jucs05.pdf
- [ ] Constant folding directly in the compiler
- [ ] Implement a JIT compiler
- [ ] Infer names of anonymous functions (Improves stack traces)

## References

- https://github.com/Janko-dev/yabil/
- https://luajit.org/luajit.html
