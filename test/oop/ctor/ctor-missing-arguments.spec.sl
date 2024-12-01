// [exit] 3
cls Foo {
  ctor(a, b) {}
}

let foo = Foo(1) // [expect-error] Uncaught error: Expected 2 arguments but got 1.
                 // [expect-error]      6 | let foo = Foo(1)
                 // [expect-error]                       ~~~
                 // [expect-error]   at line 6 at the toplevel of module "main"