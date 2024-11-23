let Nil = nil
cls Foo : Nil {} // [expect-error] Uncaught error: Base class must be a class. Was Nil.
                 // [expect-error]      2 | cls Foo : Nil {}
                 // [expect-error]                    ~~~
                 // [expect-error]   at line 2 at the toplevel of module "main"