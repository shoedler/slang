fn quotify(s) -> "\"" + s + "\""

// Indices are inclusive and exclusive, respectively.
print quotify("123".slice(1,1)) // [expect] ""
print quotify("123".slice(0,2)) // [expect] "12"
print quotify("123".slice(1,2)) // [expect] "2"
print quotify("123".slice(0,3)) // [expect] "123"

// Negative inidices in start
print quotify("123".slice(-0,2)) // [expect] "12"
print quotify("123".slice(-1,2)) // [expect] ""
print quotify("123".slice(-2,2)) // [expect] "2"
print quotify("123".slice(-3,2)) // [expect] "12"

// Negative indices in end
print quotify("123".slice(0,-0)) // [expect] ""
print quotify("123".slice(0,-1)) // [expect] "12"
print quotify("123".slice(0,-2)) // [expect] "1"

// Nil in end
print quotify("123".slice(0,nil)) // [expect] "123"
print quotify("123".slice(1,nil)) // [expect] "23"
print quotify("123".slice(2,nil)) // [expect] "3"

// If somehow start > end, it should return an empty str.
print quotify("123".slice(1,0)) // [expect] ""
print quotify("123".slice(2,1)) // [expect] ""

// If any of the indices are out of bounds, it should clamp to the bounds.
print quotify("123".slice(0,5))    // [expect] "123"
print quotify("123".slice(-4,nil)) // [expect] "123"

// If you try to slice an empty str, it should return an empty str.
print quotify("".slice(0,0)) // [expect] ""

// Non-numbers should throw an error.
print try "123".slice(0,"a") else error // [expect] Expected argument 1 of type Int but got Str.
print try "123".slice("a",0) else error // [expect] Expected argument 0 of type Int but got Str.
print try "123".slice("a","a") else error // [expect] Expected argument 0 of type Int but got Str.

// Also, floats should throw an error.
print try "123".slice(0,0.5) else error // [expect] Expected argument 1 of type Int but got Float.
print try "123".slice(0.5,0) else error // [expect] Expected argument 0 of type Int but got Float.
