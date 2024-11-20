// Passing no argument
print [1,2,3].reduce(1, 1) // [ExpectError] Uncaught error: Expected argument 1 to be callable but got Int.
                           // [ExpectError]      2 | print [1,2,3].reduce(1, 1)
                           // [ExpectError]                        ~~~~~~~~~~~~
                           // [ExpectError]   at line 2 at the toplevel of module "main"