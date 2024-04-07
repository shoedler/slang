print  fn -> 1  is Fn // [Expect] <Fn __anon>
print (fn -> 1) is Fn // [Expect] true

// Has "comparison" precedence, so preceeding comparisons get evaluated first
print 1 > 2 is Num // [Expect] false

// Precedence is higher than equality operators
print  1 == 1  is Bool // [Expect] false
print (1 == 1) is Bool // [Expect] true

// Precedence is lower than arithmetic operators
print  1 + Num("2") is Num // [Expect] true