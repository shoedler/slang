import File
print File // [Expect] <Instance of Module>

// Since this is modifying the file system, we want to check that cwd() returns something reasonable
if cwd() == "C:\\Projects\\slang\\test\\builtin\\" {
  // Valid path
  print File.write(cwd() + "a.b", "Hello world") // [Expect] true
  print File.read(cwd() + "a.b") // [Expect] Hello world
  // (We assume File.read is working correctly)

  // Overwrites the file
  print File.write(cwd() + "a.b", "Ok") // [Expect] true
  print File.read(cwd() + "a.b") // [Expect] Ok

  // Invalid path
  print File.write(cwd() + "a.b/c", "Hello world") // [Expect] false
}
