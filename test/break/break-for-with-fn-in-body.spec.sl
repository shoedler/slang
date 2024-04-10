for let i = 0; i < 3; i++; {
  let do_something = fn {
    for let j = 0; j < 3; j++; {
      if i == j 
        break
      print "  do_something: i = " + i.to_str() + ", j = " + j.to_str()
    }
  }

  if i == 3
    break

  do_something()
}

// [Expect] do_something: i = 1, j = 0
// [Expect] do_something: i = 2, j = 0
// [Expect] do_something: i = 2, j = 1