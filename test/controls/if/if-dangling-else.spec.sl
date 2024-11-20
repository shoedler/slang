// A dangling else binds to the right-most if.
if true if false print "bad" else print "good" // [Expect] good
if false if true print "bad" else print "bad"
