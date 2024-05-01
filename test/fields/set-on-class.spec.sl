cls Foo {}
Foo.bar = "value" // [ExpectError] Uncaught error: Type Class does not support property-set access.
                  // [ExpectError]      2 | Foo.bar = "value"
                  // [ExpectError]              ~~~~~~
                  // [ExpectError]   at line 2 at the toplevel of module "main"