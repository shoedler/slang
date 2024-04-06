import b from "modules/b" 
import std from "modules/c"  // [ExpectRuntimeError] Uncaught error: Could not import module 'std'. File 'modules/c' does not exist.
                             // [ExpectRuntimeError]    at line 2 at the toplevel of module "main"
                             // [Expect] Running module b