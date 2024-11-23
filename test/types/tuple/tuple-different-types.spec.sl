let a = (1, false, nil, "hello", fn -> 1, [1,2,3], (1,2,3))

print a // [expect] (1, false, nil, hello, <Fn (anon)>, [1, 2, 3], (1, 2, 3))
print a[4]() // [expect] 1