import a 
print a.msg is Str // [expect] true

import f
print f.fail is Fn // [expect] true

import Debug
import File

// Modules a/f are only imported once, by their absolute path:
let module_cache = Debug.modules()
let a_absolute_path = File.join_path(cwd(), "a.sl")
let f_absolute_path = File.join_path(cwd(), "f.sl")

print a_absolute_path in module_cache // [expect] true
print "a" in module_cache // [expect] false

print f_absolute_path in module_cache // [expect] true
print "f" in module_cache // [expect] false
