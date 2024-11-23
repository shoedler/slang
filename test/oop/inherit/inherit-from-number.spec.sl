let Number = 123
cls Foo : Number {} // [expect-error] Uncaught error: Base class must be a class. Was Int.
                    // [expect-error]      2 | cls Foo : Number {}
                    // [expect-error]                    ~~~~~~
                    // [expect-error]   at line 2 at the toplevel of module "main"