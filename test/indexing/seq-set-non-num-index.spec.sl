let a = [1,2,3,"hi"]

// Non-integer
let sqrt_of_3 = 1.73205
let almost_3 = sqrt_of_3 * sqrt_of_3
print almost_3                           // [Expect] 2.999997
print try (a[almost_3] = "?") else error // [Expect] Index must be an integer, but got a float.
print a[3]                               // [Expect] hi

// String
print try (a["3"] = "?") else error      // [Expect] Seq indices must be Nums, but got Str.
