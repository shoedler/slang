let Number = 123
cls Foo : Number {} // [ExpectRuntimeError] Uncaught error: Base class must be a class. Was Int.
                    // [ExpectRuntimeError] at line 2 at the toplevel of module "main"
