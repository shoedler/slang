let a = [1,2,3,"hi"]

// Non-integer
let sqrt_of_3 = 1.73205
let almost_3 = sqrt_of_3 * sqrt_of_3
print almost_3 // (See below)
print a[almost_3] = "?" // (See below)
print a[3] // (See below)

// String
print a["3"] = "?" // [ExpectRuntimeError] Uncaught error: Seq indices must be Nums, but got Str.
                   // [ExpectRuntimeError]   at line 11 at the toplevel of module "main"

// [Expect] 2.999997
// [Expect] nil
// [Expect] hi