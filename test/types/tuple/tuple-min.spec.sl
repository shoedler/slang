// Works for ints
print (44,3,-123, 23, 99, -0).min() // [expect] -123
print (2,2,2,2,0,0,1).min() // [expect] 0

// Works for floats
print (0.11,0.12,0.1,-0.11,-0.1,-0.12).min() // [expect] -0.12
print (0.1,0.1,0.1,0.2,0.1,0.1).min() // [expect] 0.1

// Works if the array is empty
print (,).min() // [expect] nil

// Works for types with "lt"
cls X {
  ctor(x) { this.x = x }
  fn lt(x) {
    ret this.x < x.x
  }
}
print (X(3), X(2), X(1)).min().x // [expect] 1

// ...which must be implemented correctly (i.e. return a boolean)
cls Y { fn lt(x) -> 1}
print try (Y(), Y(), Y()).min() else error // [expect] Method "Y.lt" must return a Bool. Got Int.

// ...even broader: it must be implemented at all
cls Z {}
print try (Z(), Z(), Z()).min() else error // [expect] Type Z does not support "lt".

// Works for large tuples
const a = Tuple(Seq(10000).map(fn(_, i) -> 10000-i))
print a.min() // [expect] 1

// Doesn't modify the tuple
const c = (1,2,3)
const d = c.min()
print c // [expect] (1, 2, 3)
print d // [expect] 1
