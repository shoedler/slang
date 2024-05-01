let Number = 123
cls Foo : Number {} // [ExpectError] Uncaught error: Base class must be a class. Was Int.
                    // [ExpectError]      2 | cls Foo : Number {}
                    // [ExpectError]                    ~~~~~~
                    // [ExpectError]   at line 2 at the toplevel of module "main"