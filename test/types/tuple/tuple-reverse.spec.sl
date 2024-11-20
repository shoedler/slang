// No items
print (,).reverse() // [Expect] ()

// One item
print (1,).reverse() // [Expect] (1)

// Multiple items
print (1, 2, 3).reverse() // [Expect] (3, 2, 1)
print (true, false).reverse() // [Expect] (false, true)
print ("a", "b", "c").reverse() // [Expect] (c, b, a)

// Multiple items with different types
print (1, "a", true).reverse() // [Expect] (true, a, 1)

// Reverse twice
print (1, 2, 3).reverse().reverse() // [Expect] (1, 2, 3)

// Reversing twice results in the same tuple
let a = (1, 2, 3)
print a == a.reverse() // [Expect] false
print a == a.reverse().reverse() // [Expect] true