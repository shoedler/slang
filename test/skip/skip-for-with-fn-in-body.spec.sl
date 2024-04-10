for let i = 0; i < 3; i++; {
  let do_something = fn {
    for let j = 0; j < 3; j++; {
      if i == j
        skip
      print "  do_something: i = " + i.to_str() + ", j = " + j.to_str()
    }
  }

  if i % 2 == 0
    skip

  do_something()
}

// [Expect] do_something: i = 1, j = 0
// [Expect] do_something: i = 1, j = 2