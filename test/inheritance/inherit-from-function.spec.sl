fn foo() {}

cls Subclass : foo {} // [ExpectError] Uncaught error: Base class must be a class. Was Fn.
                      // [ExpectError]      3 | cls Subclass : foo {}
                      // [ExpectError]                         ~~~
                      // [ExpectError]   at line 3 at the toplevel of module "main"