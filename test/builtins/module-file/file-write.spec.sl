import File
print File // [expect] <Instance of Module>

// Since this is modifying the file system, we want to check that cwd() returns something reasonable
if cwd() == "C:\\Projects\\slang\\test\\builtins\\module-file\\" {
  // Valid path
  print File.write(cwd() + "a.b", "Hello world") // [expect] true
  print File.read(cwd() + "a.b") // [expect] Hello world
  // (We assume File.read is working correctly)

  // Overwrites the file
  print File.write(cwd() + "a.b", "Ok") // [expect] true
  print File.read(cwd() + "a.b") // [expect] Ok

  // Invalid path
  print File.write(cwd() + "a.b/c", "Hello world") // [expect] false
}
