fn x() {
  ret 1 2 // [ExpectCompileError] Compile error at line 2 at '2': Expecting newline, '}' or some other statement after return value.
}         // [ExpectCompileError] Compile error at line 3 at end: Expecting '}' after block.