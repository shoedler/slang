cls Foo {
  ctor(a, b) {}
}

let foo = Foo(1) // [ExpectError] Uncaught error: Expected 2 arguments but got 1.
                 // [ExpectError]      5 | let foo = Foo(1)
                 // [ExpectError]                       ~~
                 // [ExpectError]   at line 5 at the toplevel of module "main"