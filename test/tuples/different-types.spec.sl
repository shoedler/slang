let a = (1, false, nil, "hello", fn -> 1, [1,2,3], (1,2,3))

print a // [Expect] (1, false, nil, hello, <Fn __anon>, [1, 2, 3], (1, 2, 3))
print a[4]() // [Expect] 1