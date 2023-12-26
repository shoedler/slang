cls A {
  fn method() {}
}

cls B : A {
  fn method() {
    (base).method() // [ExpectCompileError] Compile error at line 7 at ')': Expecting '.' after 'base'.
  }
}
// [ExpectCompileError] Compile error at line 10 at end: Expecting '}' after block.