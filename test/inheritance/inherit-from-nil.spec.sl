let Nil = nil
cls Foo : Nil {} // [ExpectRuntimeError] Base class must be a class.
                 // [ExpectRuntimeError] [line 2] in fn toplevel