let i = 0
while i++ <= 5 {
  let do_something = fn {
    let j = 0
    while j++ <= 5 {
      if j == 3 
        skip
      print "do_something: i = " + i.to_str() + ", j = " + j.to_str()
    }
  }

  for let l = 0; l <= 5; l++; {
    if l == 2 
      skip
    print "l = " + l.to_str()
  }

  let do_something_else = fn {
    let k = 0
    while k++ <= 5 {
      if k == 2 
        skip
      print "do_something_else: i = " + i.to_str() + ", k = " + k.to_str()
    }
  }

  if i % 2 == 0
    skip

  do_something()
  do_something_else()
}
// [Expect] l = 0
// [Expect] l = 1
// [Expect] l = 3
// [Expect] l = 4
// [Expect] l = 5
// [Expect] do_something: i = 1, j = 1
// [Expect] do_something: i = 1, j = 2
// [Expect] do_something: i = 1, j = 4
// [Expect] do_something: i = 1, j = 5
// [Expect] do_something_else: i = 1, k = 1
// [Expect] do_something_else: i = 1, k = 3
// [Expect] do_something_else: i = 1, k = 4
// [Expect] do_something_else: i = 1, k = 5
// [Expect] l = 0
// [Expect] l = 1
// [Expect] l = 3
// [Expect] l = 4
// [Expect] l = 5
// [Expect] l = 0
// [Expect] l = 1
// [Expect] l = 3
// [Expect] l = 4
// [Expect] l = 5
// [Expect] do_something: i = 3, j = 1
// [Expect] do_something: i = 3, j = 2
// [Expect] do_something: i = 3, j = 4
// [Expect] do_something: i = 3, j = 5
// [Expect] do_something_else: i = 3, k = 1
// [Expect] do_something_else: i = 3, k = 3
// [Expect] do_something_else: i = 3, k = 4
// [Expect] do_something_else: i = 3, k = 5
// [Expect] l = 0
// [Expect] l = 1
// [Expect] l = 3
// [Expect] l = 4
// [Expect] l = 5
// [Expect] l = 0
// [Expect] l = 1
// [Expect] l = 3
// [Expect] l = 4
// [Expect] l = 5
// [Expect] do_something: i = 5, j = 1
// [Expect] do_something: i = 5, j = 2
// [Expect] do_something: i = 5, j = 4
// [Expect] do_something: i = 5, j = 5
// [Expect] do_something_else: i = 5, k = 1
// [Expect] do_something_else: i = 5, k = 3
// [Expect] do_something_else: i = 5, k = 4
// [Expect] do_something_else: i = 5, k = 5