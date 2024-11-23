// Passing no argument
print [1,2,3].reduce(1, 1) // [expect-error] Uncaught error: Expected argument 1 to be callable but got Int.
                           // [expect-error]      2 | print [1,2,3].reduce(1, 1)
                           // [expect-error]                        ~~~~~~~~~~~~
                           // [expect-error]   at line 2 at the toplevel of module "main"