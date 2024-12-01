// [exit] 3
cls Foo {
  fn method(a, b) {}
}

Foo().method(1) // [expect-error] Uncaught error: Expected 2 arguments but got 1.
                // [expect-error]      6 | Foo().method(1)
                // [expect-error]                ~~~~~~~~~
                // [expect-error]   at line 6 at the toplevel of module "main"