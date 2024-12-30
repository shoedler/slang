for let a = 0; a < 3; a++; {
  for let b = 0; b < 3; b++; {
    for let c = 0; c < 3; c++; {
      if a > 0 or b > 0 or c > 0
        break

      const x = 1
      const y = 2
      const z = 3

      print x+y+z
    }
  }
}
// [expect] 6

fn foo {
  for let a = 0; a < 3; a++; {
    for let b = 0; b < 3; b++; {
      for let c = 0; c < 3; c++; {
        if a > 0 or b > 0 or c > 0
          break

        const x = 1
        const y = 2
        const z = 3

        print x+y+z
      }
    }
  }
}
foo()
// [expect] 6
