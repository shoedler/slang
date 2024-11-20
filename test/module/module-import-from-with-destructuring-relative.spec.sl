import { nothing } from "modules/b" // [Expect] Running module b
print nothing // [Expect] nil

import { x } from "modules/b" // Should not run module b again
print x // [Expect] 42

import { y } from "modules/c"  // [ExpectError] Uncaught error: Could not import module 'modules/c'. File 'modules/c' does not exist.
                               // [ExpectError]      7 | import { y } from "modules/c"
                               // [ExpectError]                            ~~~~~~~~~~~
                               // [ExpectError]   at line 7 at the toplevel of module "main"