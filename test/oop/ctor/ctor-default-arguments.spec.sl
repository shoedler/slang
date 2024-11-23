cls Foo {}

let foo = Foo(1, 2, 3) // [expect-error] Uncaught error: Expected 0 arguments but got 3.
                       // [expect-error]      3 | let foo = Foo(1, 2, 3)
                       // [expect-error]                       ~~~~~~~~~
                       // [expect-error]   at line 3 at the toplevel of module "main"