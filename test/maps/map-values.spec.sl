let m = {1:2, 3:4, 5:6}

// Sadly, the order is not guaranteed
print m.values() // [Expect] [6, 4, 2]
print m[7] = 8 // [Expect] 8
print m.values() // [Expect] [6, 4, 2, 8]