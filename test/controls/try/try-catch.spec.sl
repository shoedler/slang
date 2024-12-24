// Basic try-catch
try {
  nil/2
}
catch {
  print error // [expect] Type Nil does not support 'div'.
}
print "still running" // [expect] still running

// Catch is optional
try { nil/2 }
print "still running" // [expect] still running

// Doesn't need to be a block
try nil/2 catch print error // [expect] Type Nil does not support 'div'.
print "still running" // [expect] still running