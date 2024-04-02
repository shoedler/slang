print "".trim().len()           // [Expect] 0
print " ".trim().len()          // [Expect] 0
print "          ".trim().len() // [Expect] 0
print "    h ".trim() + "ello"  // [Expect] hello

print "right ".trim() + 
      "  left".trim()  + 
      "end"                     // [Expect] rightleftend

print "\f TEST \f".trim()       // [Expect] TEST
print "\n TEST \n".trim()       // [Expect] TEST
print "\r TEST \r".trim()       // [Expect] TEST
print "\t TEST \t".trim()       // [Expect] TEST
print "\v TEST \v".trim()       // [Expect] TEST
print "\" TEST \"".trim()       // [Expect] " TEST "