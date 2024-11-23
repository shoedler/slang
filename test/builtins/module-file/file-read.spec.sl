// Hello! Lol
import File
print File // [expect] <Instance of Module>

// Valid path
let self = File.read(cwd() + "file-read.spec.sl")
print self.split(" ")[1] // [expect] Hello!

// Invalid path
print try File.read(cwd() + "this-file-does-not-exist.spec.sl") else error // [expect] File 'C:\Projects\slang\test\builtins\module-file\this-file-does-not-exist.spec.sl' does not exist.
