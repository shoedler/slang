fn all_ints(seq) -> seq.every(fn(x) -> x is Int)

// Basic cases
print "123".ints() // [expect] [123]
print "123 456".ints() // [expect] [123, 456]
print "1.2.3,4".ints() // [expect] [1, 2, 3, 4]

let ints = "1.234".ints()
print ints // [expect] [1, 234]
print all_ints(ints) // [expect] true

ints = "1,234".ints()
print ints // [expect] [1, 234]
print all_ints(ints) // [expect] true

// Negative numbers
ints = "-1,234".ints()
print ints // [expect] [-1, 234]
print all_ints(ints) // [expect] true

ints = "-1.234".ints()
print ints // [expect] [-1, 234]
print all_ints(ints) // [expect] true

// Leading/trailing whitespace
print "  1  23-4  ".ints() // [expect] [1, 23, -4]
print "1
2
3
-4 5".ints() // [expect] [1, 2, 3, -4, 5]

// Edge cases
print "".ints() // [expect] []
