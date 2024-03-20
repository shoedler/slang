let m = {1:2, 3:4, 5:6}

// Sadly, the order is not guaranteed
print m.keys() // [Expect] [5, 3, 1]
print m[7] = 8 // [Expect] 8
print m.keys() // [Expect] [5, 3, 1, 7]