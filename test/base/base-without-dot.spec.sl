cls A {}

cls B : A {
  fn method() { // [Exit] 2
    base // [ExpectError] Compile error at line 6 at '}': Expecting '.' after 'base'.
  }
}
// [ExpectError] Compile error at line 8 at end: Expecting '}' after block.