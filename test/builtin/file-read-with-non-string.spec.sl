import File

print File.read(1)   // [ExpectRuntimeError] Uncaught error: Expected argument 0 of type Str but got Num.
                     // [ExpectRuntimeError]   at line 3 in native "read"
                     // [ExpectRuntimeError]   at line 3 at the toplevel of module "main"