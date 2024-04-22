let obj = {1:2, 3:4, 5:6}

// Sadly, the order is not guaranteed
print obj.values() // [Expect] [2, 4, 6]
print obj[7] = 8 // [Expect] 8
print obj.values() // [Expect] [2, 4, 6, 8]

// Fuzzy tests
print {nil:nil}.values()               // [Expect] [nil]
print {nil:1}.values()                 // [Expect] [1]
print {"":""}.values()                 // [Expect] []
print {{1:1}.values(): {2:2}.values()} // [Expect] {[1]: [2]}