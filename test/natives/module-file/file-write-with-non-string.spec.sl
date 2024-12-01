// [exit] 3
import File

print File.write(1, "")  // [expect-error] Uncaught error: Expected argument 0 of type Str but got Int.
                         // [expect-error]      4 | print File.write(1, "")
                         // [expect-error]                     ~~~~~~~~~~~~
                         // [expect-error]   at line 4 at the toplevel of module "main"