let Nil = nil
cls Foo : Nil {} // [ExpectRuntimeError] Base class must be a class.
                 // [ExpectRuntimeError] at line 2 at the toplevel