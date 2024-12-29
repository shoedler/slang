// With meaningful newlines, this is now allowed:
let a = [1,2,3]
[4].map(fn (x) { print x }) // [expect] 4

// Where it would've been read as:
print try a = [1,2,3][4].map(fn (x) { print x }) else error // [expect] Undefined callable 'map' in type Nil.
// before meaningful newlines were added.

// Also, chaining method calls kinda breaks the rule of one statement per line. But it's an exception:
print {1:10, true:10, nil:10, []:10, "9": "10"}
  .values()
  .fold("", fn (acc, x) -> acc + x) // [expect] 1010101010

// This is also allowed:
let i = 0
let sum = 0
sum = sum + 1
++i // Should be parsed as unary
print sum // [expect] 1
print i // [expect] 1