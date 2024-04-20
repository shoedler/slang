let a = [1,2,3,"hi"]

// Non-integer
let sqrt_of_3 = 1.73205
let almost_3 = sqrt_of_3 * sqrt_of_3
print almost_3                   // [Expect] 2.999997
print a[almost_3]                // [Expect] nil

// String
print try a["3"] else error      // [Expect]  Type Seq does not support get-indexing with Str.
