let a = { "a": 1 }

// Indexing / subscripting
print a["a"]     // [expect] 1
print a["a"] = 2 // [expect] 2

// Member access / dot notation
print a.a        // [expect] 2
print a.a = 3    // [expect] 3

print a         // [expect] {a: 3}
