let obj = {1:2, 3:4, 5:6}

// Sadly, the order is not guaranteed
print obj.keys() // [Expect] [5, 3, 1]
print obj[7] = 8 // [Expect] 8
print obj.keys() // [Expect] [5, 3, 1, 7]

// Fuzzy tests
print {nil:nil}.keys()             // [Expect] [nil]
print {nil:1}.keys()               // [Expect] [nil]
print {"":""}.keys()               // [Expect] []
print {{1:1}.keys(): {2:2}.keys()} // [Expect] {[1]: [2]}