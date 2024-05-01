import File

print File.exists(1) // [ExpectError] Uncaught error: Expected argument 0 of type Str but got Int.
                     // [ExpectError]      3 | print File.exists(1)
                     // [ExpectError]                     ~~~~~~~~~
                     // [ExpectError]   at line 3 at the toplevel of module "main"