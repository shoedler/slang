print nil.slice(0, 0) // [ExpectError] Uncaught error: Undefined callable 'slice' in type Nil.
                      // [ExpectError]      1 | print nil.slice(0, 0)
                      // [ExpectError]                    ~~~~~~~~~~~
                      // [ExpectError]   at line 1 at the toplevel of module "main"