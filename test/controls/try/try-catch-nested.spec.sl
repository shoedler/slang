try {
  print "Level one" // [expect] Level one
  try {
    print "Level two"  // [expect] Level two
    throw "Thrown exception"
  }
  catch {
    print "Caught in level two" // [expect] Caught in level two
    throw error
  }
} 
catch {
  print "Caught in level one!" // [expect] Caught in level one!
}
