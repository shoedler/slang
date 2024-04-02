import File
print File // [Expect] <Instance of Module>

// Valid path
print File.exists(cwd() + "file-exists.spec.sl") // [Expect] true

// Invalid path
print File.exists(cwd() + "this-file-does-not-exist.spec.sl") // [Expect] false

