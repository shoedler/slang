cls Foo {
  fn method(a, b) {
    print a
    print b
  }
}

Foo().method(1, 2, 3, 4) // [ExpectRuntimeError] Expected 2 arguments but got 4.
                         // [ExpectRuntimeError] [line 8] in fn toplevel