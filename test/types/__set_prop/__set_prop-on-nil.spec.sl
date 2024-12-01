// [exit] 3
nil.foo = "value" // [expect-error] Uncaught error: Type Nil does not support property-set access.
                  // [expect-error]      2 | nil.foo = "value"
                  // [expect-error]              ~~~~~~~~~~~~~
                  // [expect-error]   at line 2 at the toplevel of module "main"