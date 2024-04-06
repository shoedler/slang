fn test() {
  throw "error"
}

fn lol() {
  let a = 0
  try a = test()
  catch a = 1
  ret a;
}

print lol() // [Expect] 1