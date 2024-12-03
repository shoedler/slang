for let i = 0; i < 3; i++; {
  let do_something = fn {
    for let j = 0; j < 3; j++; {
      if i == j
        skip
      print "  do_something: i = " + i + ", j = " + j
    }
  }

  if i % 2 == 0
    skip

  do_something()
}

// [expect]   do_something: i = 1, j = 0
// [expect]   do_something: i = 1, j = 2