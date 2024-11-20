// No items
print (,).concat((,)) // [Expect] ()

// Items in receiver, no items in argument
print (1,2,3).concat((,)) // [Expect] (1, 2, 3)

// No items in receiver, items in argument
print (,).concat((1,2,3)) // [Expect] (1, 2, 3)

// Items in both receiver and argument
print (1,2,3).concat((4,5,6)) // [Expect] (1, 2, 3, 4, 5, 6)

// Nested arrays
print (1,2,3).concat((4,5,(6,7))) // [Expect] (1, 2, 3, 4, 5, (6, 7))