import File

print File.join_path("a", 1) // [ExpectError] Uncaught error: Expected argument 1 of type Str but got Int.
                             // [ExpectError]      3 | print File.join_path("a", 1)
                             // [ExpectError]                     ~~~~~~~~~~~~~~~~~
                             // [ExpectError]   at line 3 at the toplevel of module "main"