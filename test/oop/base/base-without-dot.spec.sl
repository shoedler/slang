cls A {}

cls B : A {
  fn method() { // [exit] 2
    base // [expect-error] Parser error at line 6 at '}': Expected '.' or '('. after 'base'.
  }      // [expect-error]      6 |   }
}        // [expect-error]            ~