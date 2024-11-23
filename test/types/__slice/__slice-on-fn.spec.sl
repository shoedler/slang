print (fn -> 1).slice(0, 0) // [expect-error] Uncaught error: Undefined callable 'slice' in type Fn.
                            // [expect-error]      1 | print (fn -> 1).slice(0, 0)
                            // [expect-error]                          ~~~~~~~~~~~
                            // [expect-error]   at line 1 at the toplevel of module "main"