fn quotify(s) -> "\"" + s + "\""

// Indices are inclusive and exclusive, respectively.
print quotify("123".slice(1,1)) // [Expect] ""
print quotify("123".slice(0,2)) // [Expect] "12"
print quotify("123".slice(1,2)) // [Expect] "2"
print quotify("123".slice(0,3)) // [Expect] "123"

// Negative inidices in start
print quotify("123".slice(-0,2)) // [Expect] "12"
print quotify("123".slice(-1,2)) // [Expect] ""
print quotify("123".slice(-2,2)) // [Expect] "2"
print quotify("123".slice(-3,2)) // [Expect] "12"

// Negative indices in end
print quotify("123".slice(0,-0)) // [Expect] ""
print quotify("123".slice(0,-1)) // [Expect] "12"
print quotify("123".slice(0,-2)) // [Expect] "1"

// Nil in end
print quotify("123".slice(0,nil)) // [Expect] "123"
print quotify("123".slice(1,nil)) // [Expect] "23"
print quotify("123".slice(2,nil)) // [Expect] "3"

// If somehow start > end, it should return an empty str.
print quotify("123".slice(1,0)) // [Expect] ""
print quotify("123".slice(2,1)) // [Expect] ""

// If any of the indices are out of bounds, it should clamp to the bounds.
print quotify("123".slice(0,5))    // [Expect] "123"
print quotify("123".slice(-4,nil)) // [Expect] "123"

// If you try to slice an empty str, it should return an empty str.
print quotify("".slice(0,0)) // [Expect] ""

// Non-numbers should throw an error.
print try "123".slice(0,"a") else error // [Expect] Expected argument 1 of type Num but got Str.
print try "123".slice("a",0) else error // [Expect] Expected argument 0 of type Num but got Str.
print try "123".slice("a","a") else error // [Expect] Expected argument 0 of type Num but got Str.

// Also, floats should throw an error.
print try "123".slice(0,0.5) else error // [Expect] Indices must be integers, but got floats.
print try "123".slice(0.5,0) else error // [Expect] Indices must be integers, but got floats.
