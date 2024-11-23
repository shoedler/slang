fn x() {  // [exit] 2
  ret 1 2 // [expect-error] Compile error at line 2 at '2': Expecting newline, '}' or some other statement after return value.
}         // [expect-error] Compile error at line 3 at end: Expecting '}' after block.