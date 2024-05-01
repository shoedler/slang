import b from "modules/b" 
import std from "modules/c"  // [ExpectError] Uncaught error: Could not import module 'std'. File 'modules/c' does not exist.
                             // [ExpectError]      2 | import std from "modules/c"
                             // [ExpectError]          ~~~~~~~~~~~~~~~~
                             // [ExpectError]   at line 2 at the toplevel of module "main"
                             // [Expect] Running module b