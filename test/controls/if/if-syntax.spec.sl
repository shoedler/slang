// Evaluate the 'then' expression if the condition is true.
if true print "good" // [Expect] good
if false print "bad"

// Allow block body.
if true { print "block" } // [Expect] block

// Assignment in if condition.
let a = false
if a = true print a // [Expect] true
