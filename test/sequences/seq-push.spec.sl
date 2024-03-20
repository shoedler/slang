let s = []
print s.push(1) // [Expect] nil
print s // [Expect] [1]

s.push("hello", nil, fn -> 1, true)

print s // [Expect] [1, hello, nil, <Fn <Anon>>, true]