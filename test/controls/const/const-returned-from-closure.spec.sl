let f

{
  const local = 1
  fn fun() -> local
  
  let r = fun()
  r++

  print r // [Expect] 2

  f = fun
}

let r = f()
r++
print r // [Expect] 2
