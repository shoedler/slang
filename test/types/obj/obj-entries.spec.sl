let obj = {1:2, 3:4, 5:6}

// Sadly, the order is not guaranteed
print obj.entries() // [expect] [[1, 2], [3, 4], [5, 6]]
print obj[7] = 8 // [expect] 8
print obj.entries() // [expect] [[1, 2], [3, 4], [5, 6], [7, 8]]

// Fuzzy tests
print {nil:nil}.entries()                // [expect] [[nil, nil]]
print {nil:1}.entries()                  // [expect] [[nil, 1]]
print {"":""}.entries()                  // [expect] [[, ]]
print {{1:1}.entries(): {2:2}.entries()} // [expect] {[[1, 1]]: [[2, 2]]}

