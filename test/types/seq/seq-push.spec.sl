let s = []
print s.push(1) // [expect] nil
print s // [expect] [1]

s.push("hello", nil, fn -> 1, true)

print s // [expect] [1, hello, nil, <Fn $anon_fn$>, true]