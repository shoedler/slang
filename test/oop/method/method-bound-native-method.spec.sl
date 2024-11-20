let seq = [1,2,3]

let f = seq.to_str // Bind the method to a variable
print typeof(f)   // [Expect] <Class Fn>
print f()         // [Expect] [1, 2, 3]