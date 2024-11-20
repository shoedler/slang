// False and nil are false.
if false print "bad" else print "false" // [Expect] false
if nil print "bad" else print "nil" // [Expect] nil

// Everything else is true.
if true print true // [Expect] true
if 0 print 0 // [Expect] 0
if "" print "empty" // [Expect] empty
