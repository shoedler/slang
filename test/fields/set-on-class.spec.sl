cls Foo {}
Foo.bar = "value" // [ExpectRuntimeError] Only instances can have fields.
                  // [ExpectRuntimeError] [line 2] in fn toplevel
