for let i = 0 ; i < 5 ; i = i + 1; {
  print i
}
// [Expect] 0
// [Expect] 1
// [Expect] 2
// [Expect] 3
// [Expect] 4

let b = false
for b ; !b; ; { // Incrementing is optional
  print b
  b = true
}
print b
// [Expect] false
// [Expect] true

let c = 0
while b {
  c = c + 1
  b = c < 5
  print c
}
// [Expect] 1
// [Expect] 2
// [Expect] 3
// [Expect] 4
// [Expect] 5

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
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] outside
// [Expect] 0
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] outside
// [Expect] 1
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] outside
// [Expect] 2
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] outside
// [Expect] 3
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] outside
// [Expect] 4
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] outside
// [Expect] 5
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] outside
// [Expect] 6
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] outside
// [Expect] 7
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] outside
// [Expect] 8
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] Hello World
// [Expect] outside
// [Expect] 9