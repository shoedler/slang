// No items will return nil
print [].yank(0) // [expect] nil

// So does a out of bound index
print [1, 2, 3].yank(3) // [expect] nil

// Removing the first item will return the item
print [1, 2, 3].yank(0) // [expect] 1

// It does modify the original array
let a = [1, 2, 3]
print a.yank(2) // [expect] 3
print a // [expect] [1, 2]

print a.yank(0) // [expect] 1
print a // [expect] [2]

print a.yank(0) // [expect] 2
print a // [expect] []