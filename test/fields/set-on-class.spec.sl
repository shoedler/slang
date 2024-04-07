cls Foo {}
Foo.bar = "value" // [ExpectRuntimeError] Uncaught error: Cannot set field 'bar' on value of type Class.
                  // [ExpectRuntimeError] at line 2 at the toplevel of module "main"
