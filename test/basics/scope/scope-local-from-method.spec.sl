let foo = "variable"

cls Foo {
  fn method {
    print foo
  }
}

Foo().method() // [expect] variable
