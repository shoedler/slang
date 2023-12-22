fn foo(a) {
  let a // [ExpectCompileError] ERROR at [line 2] at 'a': Already a variable with this name in this scope.
}
// [ExpectCompileError] ERROR at [line 4] at end: Expecting '}' after block.