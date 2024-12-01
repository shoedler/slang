// [exit] 3
let Number = 123
cls Foo : Number {} // [expect-error] Uncaught error: Base class must be a class. Was Int.
                    // [expect-error]      3 | cls Foo : Number {}
                    // [expect-error]                    ~~~~~~
                    // [expect-error]   at line 3 at the toplevel of module "main"