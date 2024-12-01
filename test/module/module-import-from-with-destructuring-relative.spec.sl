// [exit] 3
import { nothing } from "modules/b" // [expect] Running module b
print nothing // [expect] nil

import { x } from "modules/b" // Should not run module b again
print x // [expect] 42

import { y } from "modules/c"  // [expect-error] Uncaught error: Could not import module 'modules/c'. File 'modules/c' does not exist.
                               // [expect-error]      8 | import { y } from "modules/c"
                               // [expect-error]                            ~~~~~~~~~~~
                               // [expect-error]   at line 8 at the toplevel of module "main"