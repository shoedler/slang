cls Foo {}

let foo = Foo(1, 2, 3) // [ExpectError] Uncaught error: Expected 0 arguments but got 3.
                       // [ExpectError]      3 | let foo = Foo(1, 2, 3)
                       // [ExpectError]                       ~~~~~~~~
                       // [ExpectError]   at line 3 at the toplevel of module "main"