print [].to_str()         // [Expect] []
print [1, 2, 3].to_str()  // [Expect] [1, 2, 3]
print [
  true,
  true.to_str(),
  false,
  false.to_str(),
  nil,
  nil.to_str(),
].to_str()  // [Expect] [true, true, false, false, nil, nil]
print [
  (fn -> 1),
  (fn -> 1).to_str(),
  {1:2},
  {1:2}.to_str(),
  [1, 2, 3],
  [1, 2, 3].to_str(),
] // [Expect] [<Fn __anon>, <Fn __anon>, {1: 2}, {1: 2}, [1, 2, 3], [1, 2, 3]]