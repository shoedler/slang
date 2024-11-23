cls Foo {}
Foo.bar = "value" // [expect-error] Uncaught error: Type Class does not support property-set access.
                  // [expect-error]      2 | Foo.bar = "value"
                  // [expect-error]              ~~~~~~~~~~~~~
                  // [expect-error]   at line 2 at the toplevel of module "main"