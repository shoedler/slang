import { nothing } from "modules/b" // [Expect] Running module b
print nothing // [Expect] nil

import { x } from "modules/b" // Should not run module b again
print x // [Expect] 42