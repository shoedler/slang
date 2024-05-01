cls Foo {
  fn method(a, b) {
    print a
    print b
  }
}

Foo().method(1, 2, 3, 4) // [ExpectError] Uncaught error: Expected 2 arguments but got 4.
                         // [ExpectError]      8 | Foo().method(1, 2, 3, 4)
                         // [ExpectError]                ~~~~~~~~~~~~~~~~~
                         // [ExpectError]   at line 8 at the toplevel of module "main"