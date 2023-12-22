let Number = 123
cls Foo : Number {} // [ExpectRuntimeError] Base class must be a class.
                    // [ExpectRuntimeError] [line 2] in fn toplevel
