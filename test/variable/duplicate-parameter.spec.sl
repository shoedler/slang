fn foo(arg, arg) { // [ExpectCompileError] ERROR at [line 1] at 'arg': Already a variable with this name in this scope.
  "body"
} // [ExpectCompileError] ERROR at [line 3] at end: Expecting '}' after block.