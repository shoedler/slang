import File
print File // [expect] <Instance of Module>

// Valid path
print File.exists(cwd() + "file-exists.spec.sl") // [expect] true

// Invalid path
print File.exists(cwd() + "this-file-does-not-exist.spec.sl") // [expect] false

