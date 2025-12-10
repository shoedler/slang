// Works for ints
print (44,3,-123, 23, 99, -0).max() // [expect] 99
print (2,2,2,2,0,0,1).max() // [expect] 2

// Works for floats
print (0.11,0.12,0.1,-0.11,-0.1,-0.12).max() // [expect] 0.12
print (0.1,0.1,0.1,0.2,0.1,0.1).max() // [expect] 0.2

// Works if the array is empty
print (,).max() // [expect] nil

// Works for types with "gt"
cls X {
  ctor(x) { this.x = x }
  fn gt(x) {
    ret this.x > x.x
  }
}
print (X(3), X(2), X(1)).max().x // [expect] 3

// ...which must be implemented correctly (i.e. return a boolean)
cls Y { fn gt(x) -> 1}
print try (Y(), Y(), Y()).max() else error // [expect] Method "Y.gt" must return a Bool. Got Int.

// ...even broader: it must be implemented at all
cls Z {}
print try (Z(), Z(), Z()).max() else error // [expect] Type Z does not support "gt".

// Works for large tuples
const a = Tuple(Seq(10000).map(fn(_, i) -> 10000-i))
print a.max() // [expect] 10000

// Doesn't modify the tuple
const c = (1,2,3)
const d = c.max()
print c // [expect] (1, 2, 3)
print d // [expect] 3
