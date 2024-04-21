// No items will return nil
print [].remove_at(0) // [Expect] nil

// So does a out of bound index
print [1, 2, 3].remove_at(3) // [Expect] nil

// Removing the first item will return the item
print [1, 2, 3].remove_at(0) // [Expect] 1

// It does modify the original array
let a = [1, 2, 3]
print a.remove_at(2) // [Expect] 3
print a // [Expect] [1, 2]

print a.remove_at(0) // [Expect] 1
print a // [Expect] [2]

print a.remove_at(0) // [Expect] 2
print a // [Expect] []