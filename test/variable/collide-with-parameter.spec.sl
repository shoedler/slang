fn foo(a) {
  let a // [ExpectCompileError] Compile error at line 2 at 'a': Already a variable with this name in this scope.
}
// [ExpectCompileError] Compile error at line 4 at end: Expecting '}' after block.