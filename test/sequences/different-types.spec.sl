let a = [1, false, nil, "hello", fn -> 1, [1,2,3]]

print a // [Expect] {Seq 6 [1, false, nil, hello, [Fn <Anon>, arity 0], {Seq 3 [1, 2, 3]}]}
print a[4]() // [Expect] 1