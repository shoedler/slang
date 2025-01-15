print  fn -> 1  is Fn // [expect] <Fn $anon_fn$>
print (fn -> 1) is Fn // [expect] true

// Has "comparison" precedence, so preceeding comparisons get evaluated first
print 1 > 2 is Num // [expect] false

// Precedence is higher than equality operators
print  1 == 1  is Bool // [expect] false
print (1 == 1) is Bool // [expect] true

// Precedence is lower than arithmetic operators
print  1 + Int("2") is Num // [expect] true