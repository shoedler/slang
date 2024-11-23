fn foo() {}

cls Subclass : foo {} // [expect-error] Uncaught error: Base class must be a class. Was Fn.
                      // [expect-error]      3 | cls Subclass : foo {}
                      // [expect-error]                         ~~~
                      // [expect-error]   at line 3 at the toplevel of module "main"