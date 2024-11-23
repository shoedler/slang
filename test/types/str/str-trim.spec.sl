print "".trim().len           // [expect] 0
print " ".trim().len          // [expect] 0
print "          ".trim().len // [expect] 0
print "    h ".trim() + "ello"  // [expect] hello

print "right ".trim() + 
      "  left".trim()  + 
      "end"                     // [expect] rightleftend

print "\f TEST \f".trim()       // [expect] TEST
print "\n TEST \n".trim()       // [expect] TEST
print "\r TEST \r".trim()       // [expect] TEST
print "\t TEST \t".trim()       // [expect] TEST
print "\v TEST \v".trim()       // [expect] TEST
print "\" TEST \"".trim()       // [expect] " TEST "