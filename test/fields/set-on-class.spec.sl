cls Foo {}
Foo.bar = "value" // [ExpectRuntimeError] Only instances can have fields.
                  // [ExpectRuntimeError] at line 2 at the toplevel
