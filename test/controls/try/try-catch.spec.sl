// Basic try-catch
try {
  a / 0
}
catch {
  print error // [Expect] Undefined variable 'a'.
}
print "still running" // [Expect] still running

// Catch is optional
try { a / 0 }
print "still running" // [Expect] still running

// Doesn't need to be a block
try a / 0 catch print error // [Expect] Undefined variable 'a'.
print "still running" // [Expect] still running