let i = 0
while i++ < 3 {
  let do_something = fn {
    let j = 0
    while j++ < 3 {
      if i == j
        skip
      print "  do_something: i = " + i.to_str() + ", j = " + j.to_str()
    }
  }

  if i % 2 == 0
    skip

  do_something()
}

// [Expect]   do_something: i = 1, j = 2