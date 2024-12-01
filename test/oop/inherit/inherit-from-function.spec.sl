// [exit] 3
fn foo() {}

cls Subclass : foo {} // [expect-error] Uncaught error: Base class must be a class. Was Fn.
                      // [expect-error]      4 | cls Subclass : foo {}
                      // [expect-error]                         ~~~
                      // [expect-error]   at line 4 at the toplevel of module "main"