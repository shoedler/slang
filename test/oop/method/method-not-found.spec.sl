cls Foo {}

Foo().unknown() // [ExpectError] Uncaught error: Undefined callable 'unknown' in type Foo or any of its parent classes.
                // [ExpectError]      3 | Foo().unknown()
                // [ExpectError]                ~~~~~~~~~
                // [ExpectError]   at line 3 at the toplevel of module "main"