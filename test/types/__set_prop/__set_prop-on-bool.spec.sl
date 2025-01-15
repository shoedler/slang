// [exit] 3
true.foo = "value" // [expect-error] Uncaught error: Type Bool does not support property-set access.
                   // [expect-error]      2 | true.foo = "value"
                   // [expect-error]               ~~~
                   // [expect-error]   at line 2 at the toplevel of module "main"