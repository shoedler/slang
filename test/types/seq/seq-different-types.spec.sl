let a = [1, false, nil, "hello", fn -> 1, [1,2,3], (4,5,6)]

print a // [Expect] [1, false, nil, hello, <Fn (anon)>, [1, 2, 3], (4, 5, 6)]
print a[4]() // [Expect] 1