cls Foo {
  fn method(a, b) {
    print a
    print b
  }
}

Foo().method(1, 2, 3, 4) // [expect-error] Uncaught error: Expected 2 arguments but got 4.
                         // [expect-error]      8 | Foo().method(1, 2, 3, 4)
                         // [expect-error]                ~~~~~~~~~~~~~~~~~~
                         // [expect-error]   at line 8 at the toplevel of module "main"