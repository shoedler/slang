let Nil = nil
cls Foo : Nil {} // [ExpectError] Uncaught error: Base class must be a class. Was Nil.
                 // [ExpectError]      2 | cls Foo : Nil {}
                 // [ExpectError]                    ~~~
                 // [ExpectError]   at line 2 at the toplevel of module "main"