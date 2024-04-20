// Indices are inclusive and exclusive, respectively.
print [1,2,3].slice(1,1) // [Expect] []
print [1,2,3].slice(0,2) // [Expect] [1, 2]
print [1,2,3].slice(1,2) // [Expect] [2]
print [1,2,3].slice(0,3) // [Expect] [1, 2, 3]

// Negative inidices in start
print [1,2,3].slice(-0,2) // [Expect] [1, 2]
print [1,2,3].slice(-1,2) // [Expect] []
print [1,2,3].slice(-2,2) // [Expect] [2]
print [1,2,3].slice(-3,2) // [Expect] [1, 2]

// Negative indices in end
print [1,2,3].slice(0,-0) // [Expect] []
print [1,2,3].slice(0,-1) // [Expect] [1, 2]
print [1,2,3].slice(0,-2) // [Expect] [1]

// Nil in end
print [1,2,3].slice(0,nil) // [Expect] [1, 2, 3]
print [1,2,3].slice(1,nil) // [Expect] [2, 3]
print [1,2,3].slice(2,nil) // [Expect] [3]

// If somehow start > end, it should return an empty array.
print [1,2,3].slice(1,0) // [Expect] []
print [1,2,3].slice(2,1) // [Expect] []

// If any of the indices are out of bounds, it should return an empty seq.
print [1,2,3].slice(0,5)    // [Expect] []
print [1,2,3].slice(-4,nil) // [Expect] []

// If you try to slice an empty array, it should return an empty array.
print [].slice(0,0) // [Expect] []

// Non-numbers should throw an error.
print try [1,2,3].slice(0,"a") else error // [Expect] Expected argument 1 of type Num but got Str.
print try [1,2,3].slice("a",0) else error // [Expect] Expected argument 0 of type Num but got Str.
print try [1,2,3].slice("a","a") else error // [Expect] Expected argument 0 of type Num but got Str.

// Also, floats should throw an error.
print try [1,2,3].slice(0,0.5) else error // [Expect] Indices must be integers, but got floats.
print try [1,2,3].slice(0.5,0) else error // [Expect] Indices must be integers, but got floats.
