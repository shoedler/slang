cls Foo {
  ctor(a, b) {
    this.a = a
    this.b = b
  }
}

let foo = Foo(1, 2, 3, 4) // [expect-error] Uncaught error: Expected 2 arguments but got 4.
                          // [expect-error]      8 | let foo = Foo(1, 2, 3, 4)
                          // [expect-error]                       ~~~~~~~~~~~~
                          // [expect-error]   at line 8 at the toplevel of module "main"