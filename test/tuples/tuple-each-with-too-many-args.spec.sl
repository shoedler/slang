print (1,2,3).each(fn (a,b,c) -> log(a,b,c)) // [ExpectError] Uncaught error: Function passed to "each" must take 1 or 2 arguments, but got 3.
                                             // [ExpectError]      1 | print (1,2,3).each(fn (a,b,c) -> log(a,b,c))
                                             // [ExpectError]                        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                             // [ExpectError]   at line 1 at the toplevel of module "main"