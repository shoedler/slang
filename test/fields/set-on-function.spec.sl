fn foo {}

foo.bar = "value" // [ExpectRuntimeError] Closure cannot have fields.
                  // [ExpectRuntimeError] at line 3 at the toplevel
