// Works for ints
print (44,3,-123, 23, 99, -0).sort() // [expect] (-123, 0, 3, 23, 44, 99)
print (2,2,2,2,0,0,1).sort() // [expect] (0, 0, 1, 2, 2, 2, 2)

// Works for floats
print (0.11,0.12,0.1,-0.11,-0.1,-0.12).sort() // [expect] (-0.12, -0.11, -0.1, 0.1, 0.11, 0.12)
print (0.1,0.1,0.1,0.2,0.1,0.1).sort() // [expect] (0.1, 0.1, 0.1, 0.1, 0.1, 0.2)

// Works if the tuple is empty
print (,).sort() // [expect] ()

// Works for types with "lt"
cls X {
  ctor(x) { this.x = x }
  fn lt(x) {
    ret this.x < x.x
  }
}
print (X(3), X(2), X(1)).sort().map(fn (x) -> x.x) // [expect] (1, 2, 3)

// ...which must be implemented correctly (i.e. return a boolean)
cls Y { fn lt(x) -> 1}
print try (Y(), Y(), Y()).sort() else error // [expect] Method "Y.lt" must return a Bool. Got Int.

// ...even broader: it must be implemented at all
cls Z {}
print try (Z(), Z(), Z()).sort() else error // [expect] Type Z does not support "lt".

// Doesn't break hash
print (1,2,3).sort() == (1,2,3) // [expect] true
