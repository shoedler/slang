let seq = [1,2,3]

let f = seq.to_str     // Bind the method to a variable
print type_name(f)           // [Expect] BoundMethod
print f()                    // [Expect] [1, 2, 3]