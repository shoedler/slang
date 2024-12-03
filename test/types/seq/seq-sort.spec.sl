// Works for ints
print [44,3,-123, 23, 99, -0].sort() // [expect] [-123, 0, 3, 23, 44, 99]
print [2,2,2,2,0,0,1].sort() // [expect] [0, 0, 1, 2, 2, 2, 2]

// Works for floats
print [0.11,0.12,0.1,-0.11,-0.1,-0.12].sort() // [expect] [-0.12, -0.11, -0.1, 0.1, 0.11, 0.12]
print [0.1,0.1,0.1,0.2,0.1,0.1].sort() // [expect] [0.1, 0.1, 0.1, 0.1, 0.1, 0.2]

// Works if the array is empty
print [].sort() // [expect] []

// Works for types with "lt"
cls X {
  ctor(x) { this.x = x }
  fn lt(x) {
    ret this.x < x.x
  }
}
print [X(3), X(2), X(1)].sort().map(fn (x) -> x.x) // [expect] [1, 2, 3]

// ...which must be implemented correctly (i.e. return a boolean)
cls Y { fn lt(x) -> 1}
print try [Y(), Y(), Y()].sort() else error // [expect] Method "lt" must return a Bool. Got Int.

// Works for large arrays
const a = []
for let i = 100000; i > 0; i--; {
  a.push(i)
}
const b = a.sort()
print b[0] // [expect] 1
print b[b.len - 1] // [expect] 100000

// Doesn't sort in-place
const c = [1,2,3]
const d = c.sort()
print c // [expect] [1, 2, 3]
print d // [expect] [1, 2, 3]
print c == d // [expect] false
