let f1
let f2
let f3

for let i = 1 ; i < 4 ; i = i + 1; {
  let j = i
  fn f() {
    print i
    print j
  }

  if (j == 1) f1 = f
  else if (j == 2) f2 = f
  else f3 = f
}

f1() // [Expect] 4
     // [Expect] 1
f2() // [Expect] 4
     // [Expect] 2
f3() // [Expect] 4
     // [Expect] 3
