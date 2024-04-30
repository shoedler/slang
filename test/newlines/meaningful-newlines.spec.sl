// With meaningful newlines, this is now allowed:
let a = [1,2,3]
[4].map(fn (x) { print x }) // [Expect] 4

// Where it would've been read as:
print try a = [1,2,3][4].map(fn (x) { print x }) else error // [Expect] Undefined method 'map' in type Nil.
// before meaningful newlines were added.

// Also, chaining method calls kinda breaks the rule of one statement per line. But it's an exception:
print {1:10, true:10, nil:10, []:10, "9": "10"}
  .values()
  .reduce("", fn (acc, x) -> acc + x.to_str()) // [Expect] 1010101010