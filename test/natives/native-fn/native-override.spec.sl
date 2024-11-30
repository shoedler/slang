const dir = cwd() 

print dir.has("test") // [expect] true

// This will only shadow the 'cwd' function in this scope.
fn cwd -> "my/own/cwd/fn"

print cwd() // [expect] my/own/cwd/fn

import a // This uses 'cwd' under the hood, but it should use the native 'cwd' function, not the one we just defined.
print a.a // [expect] I was imported

