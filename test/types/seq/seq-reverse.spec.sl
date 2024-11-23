// No items
print [].reverse() // [expect] []

// One item
print [1].reverse() // [expect] [1]

// Multiple items
print [1, 2, 3].reverse() // [expect] [3, 2, 1]
print [true, false].reverse() // [expect] [false, true]
print ["a", "b", "c"].reverse() // [expect] [c, b, a]

// Multiple items with different types
print [1, "a", true].reverse() // [expect] [true, a, 1]

// Reverse twice
print [1, 2, 3].reverse().reverse() // [expect] [1, 2, 3]