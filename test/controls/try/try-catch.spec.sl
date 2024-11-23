// Basic try-catch
try {
  a / 0
}
catch {
  print error // [expect] Undefined variable 'a'.
}
print "still running" // [expect] still running

// Catch is optional
try { a / 0 }
print "still running" // [expect] still running

// Doesn't need to be a block
try a / 0 catch print error // [expect] Undefined variable 'a'.
print "still running" // [expect] still running