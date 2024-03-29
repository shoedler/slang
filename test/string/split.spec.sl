// No separator
print "123".split()    // [Expect] [1, 2, 3]
print "\n\r\t".split() // [Expect] [\, n, \, r, \, t]

// Edge cases
print "".split()       // [Expect] []
print "".split("")     // [Expect] []

// With separator
print "123".split("")  // [Expect] [1, 2, 3]
print "123".split("1") // [Expect] [, 23]
print "123".split("2") // [Expect] [1, 3]
print "123".split("3") // [Expect] [12, ]

// Separator matching start/end of string
print "123".split("12") // [Expect] [, 3]
print "123".split("23") // [Expect] [1, ]
print "121".split("1")  // [Expect] [, 2, ]

// Separator matches whole string
print "123".split("123") // [Expect] [, ]

// Separator not found
print "123".split("1234") // [Expect] [123]
print "123".split("4")    // [Expect] [123]