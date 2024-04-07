fn foo() {}

cls Subclass : foo {} // [ExpectRuntimeError] Uncaught error: Base class must be a class. Was Fn.
                      // [ExpectRuntimeError] at line 3 at the toplevel of module "main"