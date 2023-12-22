cls A {
  fn method() {}
}

cls B : A {
  fn method() {
    (base).method() // [ExpectCompileError] ERROR at [line 7] at ')': Expecting '.' after 'base'.
  }
}
// [ExpectCompileError] ERROR at [line 10] at end: Expecting '}' after block.