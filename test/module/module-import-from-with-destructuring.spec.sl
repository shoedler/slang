import { nothing } from "modules/b" // [expect] Running module b
print nothing // [expect] nil

import { x } from "modules/b" // Should not run module b again
print x // [expect] 42