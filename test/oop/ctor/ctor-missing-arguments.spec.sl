cls Foo {
  ctor(a, b) {}
}

let foo = Foo(1) // [expect-error] Uncaught error: Expected 2 arguments but got 1.
                 // [expect-error]      5 | let foo = Foo(1)
                 // [expect-error]                       ~~~
                 // [expect-error]   at line 5 at the toplevel of module "main"