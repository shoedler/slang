<img src="./doc/slang-dark.png" width="200">

# slang

A stack-based bytecode VM written in C, based on the book [Crafting Interpreters](https://craftinginterpreters.com/).

## References

- https://github.com/Janko-dev/yabil/

## TODOs

### Phase 1

- [x] Base implementation of the VM
- [x] Fix GC Bug in current sample script
- [x] Align fn syntax
- [x] Add tests according to the original repository of bob nystrom
- [x] `cls B < A {}` crashes the VM

### Phase 2

- [ ] Re-read the book & comment the code accordingly (Continue in chapter 26)
- [ ] Merge "function" and "functions" tests directories
- [ ] Add tests for anonymous functions
- [ ] Challenges to implement:
- [ ] 14->2: Add OP_CONSTANT_LONG. This is a must-have.
- [ ] 15->4: Single-op unaries. Not fully-fledged constant folding, but a good start.
- [ ] 16->1: String interpolation.
- [ ] 17->3: Ternaries
- [ ] 19->1: Store strings as flexible array members
- [ ] 20->1: Index a hashtable with any primitive. This is a good introduction to the goal of being able to index with reference types.
- [ ] 21->2: Constant time global variable lookup.
- [ ] 22->3: Add `const`
- [ ] 22->4: Allow more than 256 locals per scope. This is a must-have.
- [ ] 23->1: `switch` Statement
- [ ] 23->2: `continue` Statement
- [ ] 24->1: ip in register. This is a must-have.
- [ ] 25->1: Only necessary closures. Evaluate this, maybe it's not worth it.
- [ ] 25->2: Closing over the loop variable.
- [ ] 28->1: Cache `ctor` keyword. Low prio.
- [ ] 28->3: Differentiate between fields and methods textually. Use an array for lookup. Low prio.

--

### Phase 3

- [ ] Allow return stats without suffixed `;`
- [ ] Remove `OP_PRINT` completely in favor of native `print` function
- [ ] Add type conversion functions. Proposal: `int()`, `float()`, `str()`, `bool()`
- [ ] Add type checking functions. Proposal: `type()` and `Num`, `Str`, `Bool` or `@num`, `@str`, `@bool`

### Phase 4

- [ ] Implement `break`
- [ ] Implement `inc` and `dec` operators, maybe as a separate OpCode
- [ ] Implement `+=`, `-=`, `*=`, `/=`, `%=`

### Phase 5

- [ ] Implement hashtable keys for all types: Objs are by reference, primitives and strings by value
- [ ] Implement sequences

### Ideas

- [ ] Implement register-based VM https://www.lua.org/doc/jucs05.pdf
- [ ] Constant folding directly in the compiler
- [ ] Implement a JIT compiler
- [ ] Infer names of anonymous functions (Improves stack traces)
