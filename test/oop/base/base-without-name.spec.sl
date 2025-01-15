cls A {}

cls B : A {
  fn method() { // [exit] 2
    base. // [expect-error] Parser error at line 6 at '}': Expecting property or method name after '.'.
  }       // [expect-error]      6 |   }
}         // [expect-error]            ~