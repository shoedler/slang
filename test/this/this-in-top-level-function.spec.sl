fn foo() {
  this // [ExpectCompileError] ERROR at [line 2] at 'this': Can't use 'this' outside of a class.
}
// [ExpectCompileError] ERROR at [line 4] at end: Expecting '}' after block.