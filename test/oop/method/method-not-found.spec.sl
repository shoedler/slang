// [exit] 3
cls Foo {}

Foo().unknown() // [expect-error] Uncaught error: Undefined callable 'unknown' in type Foo or any of its parent classes.
                // [expect-error]      4 | Foo().unknown()
                // [expect-error]                ~~~~~~~~~
                // [expect-error]   at line 4 at the toplevel of module "main"