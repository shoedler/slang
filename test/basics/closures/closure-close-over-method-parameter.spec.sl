let f

cls Foo {
  fn method(param) {
    fn f_ { print param }
    f = f_
  }
}

Foo().method("param")
f() // [expect] param
