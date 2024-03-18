fn foo {}

foo.bar = "value" // [ExpectRuntimeError] Cannot set field 'bar' on value of type Fn.
                  // [ExpectRuntimeError] at line 3 at the toplevel of module "main"
