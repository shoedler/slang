// Edge cases
print "".split("")     // [expect] []

// With separator
print "123".split("")  // [expect] [1, 2, 3]
print "123".split("1") // [expect] [, 23]
print "123".split("2") // [expect] [1, 3]
print "123".split("3") // [expect] [12, ]

// Separator matching start/end of string
print "123".split("12") // [expect] [, 3]
print "123".split("23") // [expect] [1, ]
print "121".split("1")  // [expect] [, 2, ]

// Separator matches whole string
print "123".split("123") // [expect] [, ]

// Separator not found
print "123".split("1234") // [expect] [123]
print "123".split("4")    // [expect] [123]