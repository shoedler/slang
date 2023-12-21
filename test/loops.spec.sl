for let i = 0 ; i < 5 ; i = i + 1 {
  print i
}

let b = false
for b ; !b { // Incrementing is optional
  print b
  b = true
}

let c = 0
while b {
  c = c + 1
  b = c < 10
  print c
}

for let i = 0 ; i < 10 ; i = i + 1 {
  let outer = fn {
    let x = "outside"
    let inner = fn { print x }
    ret inner;
  }

  for let y = 0; y < 10; y = y + 1 {
    let x = fn { print "Hello World" }
    x()
  }
  let closure = outer()
  closure()
  print i
}