cls A {
  fn method() {}
}

cls B : A {
  fn method() {     // [exit] 2
    (base).method() // [expect-error] Parser error at line 7 at ')': Expected '.' or '('. after 'base'.
  }                 // [expect-error]      7 |     (base).method()
}                   // [expect-error]                   ~