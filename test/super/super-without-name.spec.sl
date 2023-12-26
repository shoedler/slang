cls A {}

cls B : A {
  fn method() {
    base. // [ExpectCompileError] Compile error at line 6 at '}': Expecting base class method name.
  }
}
// [ExpectCompileError] Compile error at line 8 at end: Expecting '}' after block.