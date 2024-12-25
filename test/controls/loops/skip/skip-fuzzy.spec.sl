let i = 0
while ++i <= 5 {
  let do_something = fn {
    let j = 0
    while ++j <= 5 {
      if j == 3 
        skip
      print "do_something: i = " + i + ", j = " + j
    }
  }

  for let l = 0; l <= 5; l++; {
    if l == 2 
      skip
    print "l = " + l
  }

  let do_something_else = fn {
    let k = 0
    while ++k <= 5 {
      if k == 2 
        skip
      print "do_something_else: i = " + i + ", k = " + k
    }
  }

  if i % 2 == 0
    skip

  do_something()
  do_something_else()
}

// [expect] l = 0
// [expect] l = 1
// [expect] l = 3
// [expect] l = 4
// [expect] l = 5
// [expect] do_something: i = 1, j = 1
// [expect] do_something: i = 1, j = 2
// [expect] do_something: i = 1, j = 4
// [expect] do_something: i = 1, j = 5
// [expect] do_something_else: i = 1, k = 1
// [expect] do_something_else: i = 1, k = 3
// [expect] do_something_else: i = 1, k = 4
// [expect] do_something_else: i = 1, k = 5
// [expect] l = 0
// [expect] l = 1
// [expect] l = 3
// [expect] l = 4
// [expect] l = 5
// [expect] l = 0
// [expect] l = 1
// [expect] l = 3
// [expect] l = 4
// [expect] l = 5
// [expect] do_something: i = 3, j = 1
// [expect] do_something: i = 3, j = 2
// [expect] do_something: i = 3, j = 4
// [expect] do_something: i = 3, j = 5
// [expect] do_something_else: i = 3, k = 1
// [expect] do_something_else: i = 3, k = 3
// [expect] do_something_else: i = 3, k = 4
// [expect] do_something_else: i = 3, k = 5
// [expect] l = 0
// [expect] l = 1
// [expect] l = 3
// [expect] l = 4
// [expect] l = 5
// [expect] l = 0
// [expect] l = 1
// [expect] l = 3
// [expect] l = 4
// [expect] l = 5
// [expect] do_something: i = 5, j = 1
// [expect] do_something: i = 5, j = 2
// [expect] do_something: i = 5, j = 4
// [expect] do_something: i = 5, j = 5
// [expect] do_something_else: i = 5, k = 1
// [expect] do_something_else: i = 5, k = 3
// [expect] do_something_else: i = 5, k = 4
// [expect] do_something_else: i = 5, k = 5