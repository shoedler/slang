// [exit] 3
import b from "modules/b" 
import std from "modules/c"  // [expect-error] Uncaught error: Could not import module 'std'. File 'modules/c' does not exist.
                             // [expect-error]      3 | import std from "modules/c"
                             // [expect-error]          ~~~~~~~~~~~~~~~~~~~~~~~~~~~
                             // [expect-error]   at line 3 at the toplevel of module "main"
                             // [expect] Running module b