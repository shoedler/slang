cls A {
  fn method() {}
}

cls B : A {
  fn method() {     // [exit] 2
    (base).method() // [expect-error] Compile error at line 7 at ')': Expecting '.' after 'base'.
  }
}
// [expect-error] Compile error at line 10 at end: Expecting '}' after block.