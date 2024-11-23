print (,).to_str()         // [expect] ()
print (1, 2, 3).to_str()  // [expect] (1, 2, 3)
print [
  true,
  true.to_str(),
  false,
  false.to_str(),
  nil,
  nil.to_str(),
].to_str()  // [expect] [true, true, false, false, nil, nil]
print (
  (fn -> 1),
  (fn -> 1).to_str(),
  {1:2},
  {1:2}.to_str(),
  (1, 2, 3),
  (1, 2, 3).to_str(),
  [1, 2, 3],
  [1, 2, 3].to_str(),
 ) // [expect] (<Fn (anon)>, <Fn (anon)>, {1: 2}, {1: 2}, (1, 2, 3), (1, 2, 3), [1, 2, 3], [1, 2, 3])