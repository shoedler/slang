cls A {}

cls B : A {
  fn method() {
    base // [ExpectCompileError] ERROR at [line 6] at '}': Expecting '.' after 'base'.
  }
}
// [ExpectCompileError] ERROR at [line 8] at end: Expecting '}' after block.