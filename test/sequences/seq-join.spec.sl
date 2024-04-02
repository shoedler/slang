// No items
print [].join("1") // Nothing, not even nil (technically it's an empty string, "")

// Single item
print [1].join("1") // [Expect] 1

// Empty separator
print [1,3,4].join("") // [Expect] 134

// Separator 
print [1,3,4].join(",") // [Expect] 1,3,4

// Differnet types
print [nil, true, 1, "string", [1,2,3], {1 : 2}].join(",") // [Expect] nil,true,1,string,[1, 2, 3],{1: 2}

