cls Foo{}
Foo[0] = 1 // [ExpectError] Uncaught error: Type Class does not support set-indexing with Int.
           // [ExpectError]      2 | Foo[0] = 1
           // [ExpectError]             ~~~~~~
           // [ExpectError]   at line 2 at the toplevel of module "main"