let m = {1:2, 3:4, 5:6}

// Sadly, the order is not guaranteed
print m.entries() // [Expect] [[5, 6], [3, 4], [1, 2]]
print m[7] = 8 // [Expect] 8
print m.entries() // [Expect] [[5, 6], [3, 4], [1, 2], [7, 8]]