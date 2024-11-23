let seq = [1,2,3]

let f = seq.to_str // Bind the method to a variable
print typeof(f)   // [expect] <Fn>
print f()         // [expect] [1, 2, 3]