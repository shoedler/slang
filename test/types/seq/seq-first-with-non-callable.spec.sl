// [exit] 3
// Passing no argument
print [1,2,3].first(1) // [expect-error] Uncaught error: Expected argument 0 to be callable but got Int.
                       // [expect-error]      3 | print [1,2,3].first(1)
                       // [expect-error]                       ~~~~~~~~~
                       // [expect-error]   at line 3 at the toplevel of module "main"