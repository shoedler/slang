import File

print File.join_path("a", 1) // [ExpectRuntimeError] Uncaught error: Expected argument 1 of type Str but got Int.
                             // [ExpectRuntimeError]   at line 3 at the toplevel of module "main"