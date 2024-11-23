fn foo(a) { // [exit] 2
  let a // [expect-error] Compile error at line 2 at 'a': Already a variable with this name in this scope.
}
// [expect-error] Compile error at line 4 at end: Expecting '}' after block.