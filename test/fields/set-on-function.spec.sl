fn foo {}

foo.bar = "value" // [ExpectRuntimeError] Uncaught error: Cannot set property 'bar' on value of type Fn.
                  // [ExpectRuntimeError] at line 3 at the toplevel of module "main"
