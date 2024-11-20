let a = { "a": 1 }

// Indexing / subscripting
print a["a"]     // [Expect] 1
print a["a"] = 2 // [Expect] 2

// Member access / dot notation
print a.a        // [Expect] 2
print a.a = 3    // [Expect] 3

print a         // [Expect] {a: 3}
