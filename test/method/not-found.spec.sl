cls Foo {}

Foo().unknown() // [ExpectRuntimeError] Undefined method 'unknown' in 'Foo' or any of its parent classes
                // [ExpectRuntimeError] at line 3 at the toplevel of module "main"