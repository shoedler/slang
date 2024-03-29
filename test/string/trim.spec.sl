print "".trim().len()           // [Expect] 0
print " ".trim().len()          // [Expect] 0
print "          ".trim().len() // [Expect] 0
print "    h ".trim() + "ello"  // [Expect] hello

print "right ".trim() + 
      "  left".trim()  + 
      "end"                     // [Expect] rightleftend

print "\f TEST \f".trim()       // [Expect] \f TEST \f
print "\n TEST \n".trim()       // [Expect] \n TEST \n
print "\r TEST \r".trim()       // [Expect] \r TEST \r
print "\t TEST \t".trim()       // [Expect] \t TEST \t
print "\v TEST \v".trim()       // [Expect] \v TEST \v
