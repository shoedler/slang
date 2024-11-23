cls Foo {}

Foo().unknown() // [expect-error] Uncaught error: Undefined callable 'unknown' in type Foo or any of its parent classes.
                // [expect-error]      3 | Foo().unknown()
                // [expect-error]                ~~~~~~~~~
                // [expect-error]   at line 3 at the toplevel of module "main"