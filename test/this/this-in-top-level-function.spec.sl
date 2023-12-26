fn foo() {
  this // [ExpectCompileError] Compile error at line 2 at 'this': Can't use 'this' outside of a class.
}
// [ExpectCompileError] Compile error at line 4 at end: Expecting '}' after block.