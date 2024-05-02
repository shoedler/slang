// [Exit] 2
fn foo(arg, arg) { // [ExpectError] Compile error at line 2 at 'arg': Already a variable with this name in this scope.
  "body"
} // [ExpectError] Compile error at line 4 at end: Expecting '}' after block.