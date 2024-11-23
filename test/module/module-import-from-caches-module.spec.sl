import b from "modules/b"
// [expect] Running module b

import { x } from "modules/b"
print "Nothing, since this modules is already imported" // [expect] Nothing, since this modules is already imported
print x // [expect] 42

import Debug
import File

// Module b is only imported once, by its absolute path:
let module_cache = Debug.modules()
let modules_dir = File.join_path(cwd(), "modules")
let b_absolute_path = File.join_path(modules_dir, "b.sl")

print b_absolute_path in module_cache // [expect] true
print "b" in module_cache // [expect] false