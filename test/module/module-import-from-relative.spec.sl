import b from "modules/b" 
import std from "modules/c"  // [expect-error] Uncaught error: Could not import module 'std'. File 'modules/c' does not exist.
                             // [expect-error]      2 | import std from "modules/c"
                             // [expect-error]          ~~~~~~~~~~~~~~~~~~~~~~~~~~~
                             // [expect-error]   at line 2 at the toplevel of module "main"
                             // [expect] Running module b