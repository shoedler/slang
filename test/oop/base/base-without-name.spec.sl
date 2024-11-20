cls A {}

cls B : A {
  fn method() { // [Exit] 2
    base. // [ExpectError] Compile error at line 6 at '}': Expecting base class method name.
  }
}
// [ExpectError] Compile error at line 8 at end: Expecting '}' after block.