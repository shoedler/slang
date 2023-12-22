fn foo {}

foo.bar = "value" // [ExpectRuntimeError] Only instances can have fields.
                  // [ExpectRuntimeError] [line 3] in fn toplevel
