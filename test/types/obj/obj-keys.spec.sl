let obj = {1:2, 3:4, 5:6}

// Sadly, the order is not guaranteed
print obj.keys() // [expect] [1, 3, 5]
print obj[7] = 8 // [expect] 8
print obj.keys() // [expect] [1, 3, 5, 7]

// Fuzzy tests
print {nil:nil}.keys()             // [expect] [nil]
print {nil:1}.keys()               // [expect] [nil]
print {"":""}.keys()               // [expect] []
print {{1:1}.keys(): {2:2}.keys()} // [expect] {[1]: [2]}