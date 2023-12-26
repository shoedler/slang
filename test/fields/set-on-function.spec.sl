fn foo {}

foo.bar = "value" // [ExpectRuntimeError] Only instances can have fields.
                  // [ExpectRuntimeError] at line 3 at the toplevel
