// Indices are inclusive and exclusive, respectively.
print (1,2,3).slice(1,1) // [expect] ()
print (1,2,3).slice(0,2) // [expect] (1, 2)
print (1,2,3).slice(1,2) // [expect] (2)
print (1,2,3).slice(0,3) // [expect] (1, 2, 3)

// Negative inidices in start
print (1,2,3).slice(-0,2) // [expect] (1, 2)
print (1,2,3).slice(-1,2) // [expect] ()
print (1,2,3).slice(-2,2) // [expect] (2)
print (1,2,3).slice(-3,2) // [expect] (1, 2)

// Negative indices in end
print (1,2,3).slice(0,-0) // [expect] ()
print (1,2,3).slice(0,-1) // [expect] (1, 2)
print (1,2,3).slice(0,-2) // [expect] (1)

// Nil in end
print (1,2,3).slice(0,nil) // [expect] (1, 2, 3)
print (1,2,3).slice(1,nil) // [expect] (2, 3)
print (1,2,3).slice(2,nil) // [expect] (3)

// If somehow start > end, it should return an empty array.
print (1,2,3).slice(1,0) // [expect] ()
print (1,2,3).slice(2,1) // [expect] ()

// If any of the indices are out of bounds, it should clamp to the bounds.
print (1,2,3).slice(0,5)    // [expect] (1, 2, 3)
print (1,2,3).slice(-4,nil) // [expect] (1, 2, 3)

// ...which can return a tuple that is congruent to the original one. Which means they are equal.
print (1,2,3).slice(-4,nil) == (1,2,3) // [expect] true

// If you try to slice an empty array, it should return an empty array.
print (,).slice(0,0) // [expect] ()

// Non-numbers should throw an error.
print try (1,2,3).slice(0,"a") else error // [expect] Expected argument 1 of type Int but got Str.
print try (1,2,3).slice("a",0) else error // [expect] Expected argument 0 of type Int but got Str.
print try (1,2,3).slice("a","a") else error // [expect] Expected argument 0 of type Int but got Str.

// Also, floats should throw an error.
print try (1,2,3).slice(0,0.5) else error // [expect] Expected argument 1 of type Int but got Float.
print try (1,2,3).slice(0.5,0) else error // [expect] Expected argument 0 of type Int but got Float.
