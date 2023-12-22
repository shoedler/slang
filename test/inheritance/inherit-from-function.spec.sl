fn foo() {}

cls Subclass : foo {} // [ExpectRuntimeError] Base class must be a class.
                      // [ExpectRuntimeError] [line 3] in fn toplevel