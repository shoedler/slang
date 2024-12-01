// No items
print [].flip() // [expect] []

// One item
print [1].flip() // [expect] [1]

// Multiple items
print [1, 2, 3].flip() // [expect] [3, 2, 1]
print [true, false].flip() // [expect] [false, true]
print ["a", "b", "c"].flip() // [expect] [c, b, a]

// Multiple items with different types
print [1, "a", true].flip() // [expect] [true, a, 1]

// Reverse twice
print [1, 2, 3].flip().flip() // [expect] [1, 2, 3]