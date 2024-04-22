// Hello! Lol
import File
print File // [Expect] <Instance of Module>

// Valid path
let self = File.read(cwd() + "file-read.spec.sl")
print self.split(" ")[1] // [Expect] Hello!

// Invalid path
print try File.read(cwd() + "this-file-does-not-exist.spec.sl") else error // [Expect] File 'C:\Projects\slang\test\builtin\this-file-does-not-exist.spec.sl' does not exist.