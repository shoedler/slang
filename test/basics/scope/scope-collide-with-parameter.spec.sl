fn foo(a) { // [Exit] 2
  let a // [ExpectError] Compile error at line 2 at 'a': Already a variable with this name in this scope.
}
// [ExpectError] Compile error at line 4 at end: Expecting '}' after block.