fn foo {}

foo.bar = "value" // [ExpectRuntimeError] Uncaught error: Type Fn does not support property-set access.
                  // [ExpectRuntimeError] at line 3 at the toplevel of module "main"
