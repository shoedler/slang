// [exit] 3
cls Foo {}
Foo.bar = "value" // [expect-error] Uncaught error: Type Class does not support property-set access.
                  // [expect-error]      3 | Foo.bar = "value"
                  // [expect-error]              ~~~~~~~~~~~~~
                  // [expect-error]   at line 3 at the toplevel of module "main"