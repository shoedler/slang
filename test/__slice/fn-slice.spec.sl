print (fn -> 1).slice(0, 0) // [ExpectError] Uncaught error: Undefined callable 'slice' in type Fn.
                            // [ExpectError]      1 | print (fn -> 1).slice(0, 0)
                            // [ExpectError]                          ~~~~~~~~~~~
                            // [ExpectError]   at line 1 at the toplevel of module "main"