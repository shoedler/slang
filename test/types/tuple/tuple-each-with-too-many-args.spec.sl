// [exit] 3
print (1,2,3).each(fn (a,b,c) -> log(a,b,c)) // [expect-error] Uncaught error: Function passed to "each" must take 1 or 2 arguments, but got 3.
                                             // [expect-error]      2 | print (1,2,3).each(fn (a,b,c) -> log(a,b,c))
                                             // [expect-error]                       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                             // [expect-error]   at line 2 at the toplevel of module "main"