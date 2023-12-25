<img src="./doc/slang-dark.png" width="200">

# slang

A bytecode VM written in C, based on the book [Crafting Interpreters](https://craftinginterpreters.com/).

## References

-   https://github.com/Janko-dev/yabil/

## TODOs

### Phase 1

-   [x] Base implementation of the VM
-   [x] Fix GC Bug in current sample script
-   [x] Align fn syntax
-   [x] Add tests according to the original repository of bob nystrom
-   [ ] Allow return stats without suffixed `;`
-   [x] `cls B < A {}` crashes the VM
-   [ ] Remove `OP_PRINT` completely in favor of native `print` function

### Phase 2

-   [ ] Re-read the book & comment the code accordingly
-   [ ] Implement all (or most) of the challenges

### Phase 3

-   [ ] Implement `break`
-   [ ] Implement `continue`
-   [ ] Implement `inc` and `dec` operators, maybe as a separate OpCode
-   [ ] Implement `+=`, `-=`, `*=`, `/=`, `%=`

### Phase 4

-   [ ] Implement hashtable keys for all types: Objs are by reference, primitives and strings by value
-   [ ] Implement sequences

### Phase 5

-   [ ] Constant folding directly in the compiler

### Phase 999

-   [ ] Implement a JIT compiler
-   [ ] Infer names of anonymous functions (Improves stack traces)

## Cleanup
