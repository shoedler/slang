for let i = 0 ; i < 5 ; i = i + 1; {
  print i
}
// [expect] 0
// [expect] 1
// [expect] 2
// [expect] 3
// [expect] 4

let b = false
for b ; !b; ; { // Incrementing is optional
  print b
  b = true
}
print b
// [expect] false
// [expect] true

let c = 0
while b {
  c = c + 1
  b = c < 5
  print c
}
// [expect] 1
// [expect] 2
// [expect] 3
// [expect] 4
// [expect] 5

for let i = 0 ; i < 10 ; i = i + 1 ; {
  let outer = fn {
    let x = "outside"
    let inner = fn { print x }
    ret inner
  }

  for let y = 0; y < 10; y = y + 1 ; {
    let x = fn { print "Hello World" }
    x()
  }
  let closure = outer()
  closure()
  print i
}
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] outside
// [expect] 0
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] outside
// [expect] 1
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] outside
// [expect] 2
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] outside
// [expect] 3
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] outside
// [expect] 4
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] outside
// [expect] 5
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] outside
// [expect] 6
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] outside
// [expect] 7
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] outside
// [expect] 8
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] Hello World
// [expect] outside
// [expect] 9