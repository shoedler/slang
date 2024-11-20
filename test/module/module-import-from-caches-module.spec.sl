import b from "modules/b"
// [Expect] Running module b

import { x } from "modules/b"
print "Nothing, since this modules is already imported" // [Expect] Nothing, since this modules is already imported
print x // [Expect] 42

import Debug
import File

// Module b is only imported once, by its absolute path:
let module_cache = Debug.modules()
let modules_dir = File.join_path(cwd(), "modules")
let b_absolute_path = File.join_path(modules_dir, "b.sl")

print b_absolute_path in module_cache // [Expect] true
print "b" in module_cache // [Expect] false