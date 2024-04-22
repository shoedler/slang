let obj = {1:2, 3:4, 5:6}

// Sadly, the order is not guaranteed
print obj.entries() // [Expect] [[1, 2], [3, 4], [5, 6]]
print obj[7] = 8 // [Expect] 8
print obj.entries() // [Expect] [[1, 2], [3, 4], [5, 6], [7, 8]]

// Fuzzy tests
print {nil:nil}.entries()                // [Expect] [[nil, nil]]
print {nil:1}.entries()                  // [Expect] [[nil, 1]]
print {"":""}.entries()                  // [Expect] [[, ]]
print {{1:1}.entries(): {2:2}.entries()} // [Expect] {[[1, 1]]: [[2, 2]]}

