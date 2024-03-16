cls Foo {}
Foo.bar = "value" // [ExpectRuntimeError] Class cannot have fields.
                  // [ExpectRuntimeError] at line 2 at the toplevel of module "main"
