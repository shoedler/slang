
let a = [1,2,3]
[4].map(fn (x) { print x }) // Currently, this compiles as a[4].map(...), which is not what we want

print {1:10, true:10, nil:10, []:10, "9": "10"}
  .values()
  .reduce("", fn (acc, x) -> acc + x.to_str()) // [Expect] 1010101010