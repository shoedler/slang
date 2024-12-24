let a = [1, false, nil, "hello", fn -> 1, [1,2,3], (4,5,6)]

print a // [expect] [1, false, nil, hello, <Fn $anon_fn$>, [1, 2, 3], (4, 5, 6)]
print a[4]() // [expect] 1