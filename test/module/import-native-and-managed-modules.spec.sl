import a 
import f
import Debug
import File

// Managed modules a/f are only imported once, by their absolute path:
let module_cache = Debug.modules()
let a_absolute_path = File.join_path(cwd(), "a.sl")
let f_absolute_path = File.join_path(cwd(), "f.sl")

print a_absolute_path in module_cache // [Expect] true
print "a" in module_cache // [Expect] false

print f_absolute_path in module_cache // [Expect] true
print "f" in module_cache // [Expect] false

// Native modules are cached by their name:
print "Debug" in module_cache // [Expect] true
print "File" in module_cache // [Expect] true