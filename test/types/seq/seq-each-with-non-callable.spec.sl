// Passing no argument
print [1,2,3].each(1) // [expect-error] Uncaught error: Expected argument 0 to be callable but got Int.
                      // [expect-error]      2 | print [1,2,3].each(1)
                      // [expect-error]                        ~~~~~~~
                      // [expect-error]   at line 2 at the toplevel of module "main"