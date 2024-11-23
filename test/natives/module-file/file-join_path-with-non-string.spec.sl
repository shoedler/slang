import File

print File.join_path("a", 1) // [expect-error] Uncaught error: Expected argument 1 of type Str but got Int.
                             // [expect-error]      3 | print File.join_path("a", 1)
                             // [expect-error]                     ~~~~~~~~~~~~~~~~~
                             // [expect-error]   at line 3 at the toplevel of module "main"