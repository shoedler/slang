cls Foo {
  fn method(a, b) {}
}

Foo().method(1) // [ExpectError] Uncaught error: Expected 2 arguments but got 1.
                // [ExpectError]      5 | Foo().method(1)
                // [ExpectError]                ~~~~~~~~~
                // [ExpectError]   at line 5 at the toplevel of module "main"