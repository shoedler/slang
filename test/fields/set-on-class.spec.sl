cls Foo {}
Foo.bar = "value" // [ExpectRuntimeError] Uncaught error: Type Class does not support property-set access.
                  // [ExpectRuntimeError] at line 2 at the toplevel of module "main"
