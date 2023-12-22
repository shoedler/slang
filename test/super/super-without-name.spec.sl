cls A {}

cls B : A {
  fn method() {
    base. // [ExpectCompileError] ERROR at [line 6] at '}': Expecting base class method name.
  }
}
// [ExpectCompileError] ERROR at [line 8] at end: Expecting '}' after block.