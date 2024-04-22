import File

print File.exists(1) // [ExpectRuntimeError] Uncaught error: Expected argument 0 of type Str but got Int.
                     // [ExpectRuntimeError]   at line 3 at the toplevel of module "main"