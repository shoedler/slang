import b from "modules/b"
// [Expect] Running module b

import { x } from "modules/b"
print "Nothing" // [Expect] Nothing
print x // [Expect] 42