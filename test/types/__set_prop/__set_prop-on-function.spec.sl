fn foo {}

foo.bar = "value" // [expect-error] Uncaught error: Type Fn does not support property-set access.
                  // [expect-error]      3 | foo.bar = "value"
                  // [expect-error]              ~~~~~~~~~~~~~
                  // [expect-error]   at line 3 at the toplevel of module "main"