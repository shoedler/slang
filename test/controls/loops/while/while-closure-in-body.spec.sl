let f1
let f2
let f3

let i = 1
while (i < 4) {
  let j = i
  fn f() { print j }

  if (j == 1) f1 = f
  else if (j == 2) f2 = f
  else f3 = f

  i = i + 1
}

f1() // [expect] 1
f2() // [expect] 2
f3() // [expect] 3
