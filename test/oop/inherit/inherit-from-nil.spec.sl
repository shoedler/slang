// [exit] 3
let Nil = nil
cls Foo : Nil {} // [expect-error] Uncaught error: Base class must be a class. Was Nil.
                 // [expect-error]      3 | cls Foo : Nil {}
                 // [expect-error]                    ~~~
                 // [expect-error]   at line 3 at the toplevel of module "main"