fn foo() {}

cls Subclass : foo {} // [ExpectRuntimeError] Base class must be a class.
                      // [ExpectRuntimeError] at line 3 at the toplevel