// Single-expression body.
let c = 0
while c < 3 print c = c + 1
// [Expect] 1
// [Expect] 2
// [Expect] 3

// Block body.
let a = 0
while (a < 3) {
  print a
  a = a + 1
}
// [Expect] 0
// [Expect] 1
// [Expect] 2

// Statement bodies.
while false if true 1 else 2
while false while true 1
while false for ;;; 1