let obj = {1:2, 3:4, 5:6}

// Sadly, the order is not guaranteed
print obj.values() // [expect] [2, 4, 6]
print obj[7] = 8 // [expect] 8
print obj.values() // [expect] [2, 4, 6, 8]

// Fuzzy tests
print {nil:nil}.values()               // [expect] [nil]
print {nil:1}.values()                 // [expect] [1]
print {"":""}.values()                 // [expect] []
print {{1:1}.values(): {2:2}.values()} // [expect] {[1]: [2]}