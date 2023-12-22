cls Foo {
  fn method(a, b) {}
}

Foo().method(1) // [ExpectRuntimeError] Expected 2 arguments but got 1.
                // [ExpectRuntimeError] [line 5] in fn toplevel