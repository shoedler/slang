fn foo() {
  this // [ExpectError] Compile error at line 2 at 'this': Can't use 'this' outside of a class.
}
// [ExpectError] Compile error at line 4 at end: Expecting '}' after block.