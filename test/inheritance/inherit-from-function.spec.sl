fn foo() {}

cls Subclass : foo {} // [ExpectRuntimeError] Base class must be a class. Was Closure.
                      // [ExpectRuntimeError] at line 3 at the toplevel