// Hello! Lol
import File
print File // [expect] <Instance of Module>

// Valid path
let self = File.read(cwd() + "file-read.spec.sl")
print self.split(" ")[1] // [expect] Hello!

// Invalid path
let err = try File.read(cwd() + "this-file-does-not-exist.spec.sl") else error
print err[-49..] // [expect] this-file-does-not-exist.spec.sl' does not exist.
