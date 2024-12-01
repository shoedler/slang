// [exit] 3
fn foo {}

foo.bar = "value" // [expect-error] Uncaught error: Type Fn does not support property-set access.
                  // [expect-error]      4 | foo.bar = "value"
                  // [expect-error]              ~~~~~~~~~~~~~
                  // [expect-error]   at line 4 at the toplevel of module "main"