let f = fn -> 10
let s = [1,2,3]
let o = {1:2}
let t = (1,2,3)

let a = {
    1: 2, 
    true: false,
    nil: nil, 
    "hello": "world",
    fn -> 1: fn -> 2, // Unreachable, because we have no reference to this function
    f: fn -> 2, // Reachable

    [1,2,3]: [4,5,6], // Unreachable, because we have no reference to this array
    s: [4,5,6], // Reachable

    (1,2,3): (4,5,6), // Also reachable, because tuples are hashable
    t: (4,5,6), // Reachable. (This actually overwrites the previous entry)

    {1:2}: {3:4}, // Unreachable, because we have no reference to this object
    o: {3:4} // Reachable
}

print a[fn -> 1] // [expect] nil
print a[[1,2,3]] // [expect] nil
print a[(1,2,3)] // [expect] (4, 5, 6)
print a[{1:2}]   // [expect] nil 
print a[f] // [expect] <Fn (anon)>
print a[s] // [expect] [4, 5, 6]
print a[t] // [expect] (4, 5, 6)
print a[o] // [expect] {3: 4}
print a["hello"] // [expect] world

// This order changes when you fiddle with hashing and other object related stuff...
// print a // {1: 2, nil: nil, true: false, [1, 2, 3]: [4, 5, 6], [1, 2, 3]: [4, 5, 6], <Fn (anon)>: <Fn (anon)>, <Fn (anon)>: <Fn (anon)>, hello: world}
// ... so we can't really test the order of the keys

let entries = a.entries()
print entries.filter(fn(x) -> (x[0] is Nil)).len    // [expect] 1
print entries.filter(fn(x) -> (x[0] is Bool)).len   // [expect] 1
print entries.filter(fn(x) -> (x[0] is Num)).len    // [expect] 1
print entries.filter(fn(x) -> (x[0] is Str)).len    // [expect] 1
print entries.filter(fn(x) -> (x[0] is Seq)).len    // [expect] 2
// We only get one tuple, because we overwrote the previous one
print entries.filter(fn(x) -> (x[0] is Tuple)).len  // [expect] 1
print entries.filter(fn(x) -> (x[0] is Fn)).len     // [expect] 2
print entries.filter(fn(x) -> (x[0] is Obj)).len    // [expect] 2
