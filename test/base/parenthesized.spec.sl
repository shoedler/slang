cls A {
  fn method() {}
}

cls B : A {
  fn method() {     // [Exit] 2
    (base).method() // [ExpectError] Compile error at line 7 at ')': Expecting '.' after 'base'.
  }
}
// [ExpectError] Compile error at line 10 at end: Expecting '}' after block.