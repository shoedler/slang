fn foo(arg, arg) { // [ExpectCompileError] Compile error at line 1 at 'arg': Already a variable with this name in this scope.
  "body"
} // [ExpectCompileError] Compile error at line 3 at end: Expecting '}' after block.