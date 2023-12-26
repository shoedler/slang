let Number = 123
cls Foo : Number {} // [ExpectRuntimeError] Base class must be a class.
                    // [ExpectRuntimeError] at line 2 at the toplevel
