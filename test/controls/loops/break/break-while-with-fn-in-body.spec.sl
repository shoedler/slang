let i = 0
while i++ < 4 {
  let do_something = fn {
    let j = 0
    while j++ < 3 {
      if i == j 
        break
      print " do_something: i = " + i.to_str() + ", j = " + j.to_str()
    }
  }

  if i == 3
    break

  do_something()
}

// [expect]  do_something: i = 2, j = 1