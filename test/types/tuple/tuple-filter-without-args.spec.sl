// Passing no argument
print (1,2,3).filter() // [expect-error] Uncaught error: Expected 1 argument but got 0.
                       // [expect-error]      2 | print (1,2,3).filter()
                       // [expect-error]                        ~~~~~~~~
                       // [expect-error]   at line 2 at the toplevel of module "main"