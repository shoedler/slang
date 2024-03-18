let Number = 123
cls Foo : Number {} // [ExpectRuntimeError] Base class must be a class. Was Num.
                    // [ExpectRuntimeError] at line 2 at the toplevel of module "main"
