let f = fn -> 10
let s = [1,2,3]
let o = {1:2}

let a = {
    1: 2, 
    true: false,
    nil: nil, 
    "hello": "world",
    fn -> 1: fn -> 2, // Unreachable, because we have no reference to this function
    f: fn -> 2, // Reachable
    [1,2,3]: [4,5,6], // Unreachable, because we have no reference to this array
    s: [4,5,6], // Reachable
    {1:2}: {3:4}, // Unreachable, because we have no reference to this object
    o: {3:4} // Reachable
}

print a[fn -> 1] // [Expect] nil
print a[[1,2,3]] // [Expect] nil
print a[{1:2}]   // [Expect] nil 
print a[f] // [Expect] <Fn __anon>
print a[s] // [Expect] [4, 5, 6]
print a[o] // [Expect] {3: 4}
print a["hello"] // [Expect] world

// This order changes when you fiddle with hashing and other object related stuff...
// print a // {1: 2, nil: nil, true: false, [1, 2, 3]: [4, 5, 6], [1, 2, 3]: [4, 5, 6], <Fn __anon>: <Fn __anon>, <Fn __anon>: <Fn __anon>, hello: world}
// ... so we can't really test the order of the keys

let entries = a.entries()
print entries.filter(fn(x) -> (x[0] is Nil)).len  // [Expect] 1
print entries.filter(fn(x) -> (x[0] is Bool)).len // [Expect] 1
print entries.filter(fn(x) -> (x[0] is Num)).len  // [Expect] 1
print entries.filter(fn(x) -> (x[0] is Str)).len  // [Expect] 1
print entries.filter(fn(x) -> (x[0] is Seq)).len  // [Expect] 2
print entries.filter(fn(x) -> (x[0] is Fn)).len   // [Expect] 2
print entries.filter(fn(x) -> (x[0] is Obj)).len  // [Expect] 10
