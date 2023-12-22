// Evaluate the 'else' expression if the condition is false.
if true print "good" else print "bad"  // [Expect] good
if false print "bad" else print "good" // [Expect] good

// Allow block body.
if false nil else { print "block" } // [Expect] block
