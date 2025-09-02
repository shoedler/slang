print  fn -> 1  is Fn // [expect] <Fn $anon_fn$>
print (fn -> 1) is Fn // [expect] true

// Has "comparison" precedence, so preceeding comparisons get evaluated first:
// evaluates as 1 > 2 --> true, which is Bool, which is not Num
print 1 > 2 is Num // [expect] false

// Precedence is higher than equality operators, so preceding comparisons get evaluated later:
// evaluates as 1 is Bool --> false, 1 == false --> false
print  1 == 1  is Bool // [expect] false
print (1 == 1) is Bool // [expect] true

// Precedence is lower than arithmetic operators, so preceding arithmetic gets evaluated first:
// evaluates as 1 + Int("2") --> 3, which is Num
print  1 + Int("2") is Num // [expect] true