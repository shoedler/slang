try {
  print "Level one" // [Expect] Level one
  try {
    print "Level two"  // [Expect] Level two
    throw "Thrown exception"
  }
  catch {
    print "Caught in level two" // [Expect] Caught in level two
    throw error
  }
} 
catch {
  print "Caught in level one!" // [Expect] Caught in level one!
}
