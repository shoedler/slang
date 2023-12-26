cls A {}

cls B : A {
  fn method() {
    base // [ExpectCompileError] Compile error at line 6 at '}': Expecting '.' after 'base'.
  }
}
// [ExpectCompileError] Compile error at line 8 at end: Expecting '}' after block.