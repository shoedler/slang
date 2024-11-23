fn foo() { // [exit] 2
  this // [expect-error] Compile error at line 2 at 'this': Can't use 'this' outside of a class.
}
// [expect-error] Compile error at line 4 at end: Expecting '}' after block.