let f = fn -> 10
let s = [1,2,3]

let a = {
    1: 2, 
    true: false,
    nil: nil, 
    "hello": "world",
    fn -> 1: fn -> 2, // Unreachable, because we have no reference to this function
    f: fn -> 2, // Reachable
    [1,2,3]: [4,5,6], // Unreachable, because we have no reference to this array
    s: [4,5,6] // Reachable
}

print a[fn -> 1] // [Expect] nil
print a[[1,2,3]] // [Expect] nil
print a[f] // [Expect] <Fn __anon>
print a[s] // [Expect] [4, 5, 6]
print a["hello"] // [Expect] world
print a // [Expect] {[1, 2, 3]: [4, 5, 6], [1, 2, 3]: [4, 5, 6], <Fn __anon>: <Fn __anon>, <Fn __anon>: <Fn __anon>, nil: nil, true: false, 1: 2, hello: world}