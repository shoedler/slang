// Passing no argument
print (1,2,3).join(1) // [expect-error] Uncaught error: Expected argument 0 of type Str but got Int.
                      // [expect-error]      2 | print (1,2,3).join(1)
                      // [expect-error]                        ~~~~~~~
                      // [expect-error]   at line 2 at the toplevel of module "main"