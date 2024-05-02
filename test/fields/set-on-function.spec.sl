fn foo {}

foo.bar = "value" // [ExpectError] Uncaught error: Type Fn does not support property-set access.
                  // [ExpectError]      3 | foo.bar = "value"
                  // [ExpectError]              ~~~~~~~~~~~~~
                  // [ExpectError]   at line 3 at the toplevel of module "main"