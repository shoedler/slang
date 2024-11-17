// -----------------------------------------
// Future features
// -----------------------------------------

// Seq
[] + "Strtoseq" = ["S", "t", "r", "t", "o", "s", "e", "q"]
[] // <- Seq
[1,2,3] + [4,5,6] = [1,2,3,4,5,6]
[1,2,3] + 4 = [1,2,3,4]

let u = [1,2,3]
  .map(fn (x) -> x+1)
  .tap(print)
  .map(fn (x) -> x+1) // [3, 4, 5]

// Match expressions for enumerables
let i = match [1,2,3] {
  [1,2,3] -> "In match context, we don't really compare references",
  [_,2,3] -> "Don't care about the first value, but it ends with 2,3",
  [_,2,...] -> "Don't care about the first value, but it is followed by 2.. and ends with n",
  [Bool, Str, Obj, Fn, Num] -> "We can match on types",
  [..., 1, ...] -> "Some element is 1",
  [...] -> {
    print "We can match on sequences of any length"
    ret "We can also return from match blocks"
  }
  some(fn (x) -> (x / 2) % 2 == 0) -> "Some element is even",
  _ -> "Default case"
}
