cls A {}

cls B : A {
  fn method() { // [exit] 2
    base. // [expect-error] Compile error at line 6 at '}': Expecting base-class method name.
  }
}
// [expect-error] Compile error at line 8 at end: Expecting '}' after block.