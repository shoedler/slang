// No items
print [].join("1") // Nothing, not even nil (technically it's an empty string, "")

// Single item
print [1].join("1") // [expect] 1

// Empty separator
print [1,3,4].join("") // [expect] 134

// Separator 
print [1,3,4].join(",") // [expect] 1,3,4

// Differnet types
print [nil, true, 1, "string", [1,2,3], {1 : 2}].join(",") // [expect] nil,true,1,string,[1, 2, 3],{1: 2}

// Side effects
let a = [1,2,3]
print a.join(a.to_str()) // [expect] 1[1, 2, 3]2[1, 2, 3]3

