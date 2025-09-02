print  fn -> 1  is not Fn // [expect] <Fn $anon_fn$>
print (fn -> 1) is not Fn // [expect] false

// Has "comparison" precedence, so preceeding comparisons get evaluated first:
// evaluates as 1 > 2 --> true, which is Bool, which is not Num
print 1 > 2 is not Num // [expect] true

// Precedence is higher than equality operators, so preceding comparisons get evaluated later:
// evaluates as 1 is not Bool --> false, 1 == false --> false
print  1 == 1  is not Bool // [expect] false
print (1 == 1) is not Bool // [expect] false

// Precedence is lower than arithmetic operators, so preceding arithmetic gets evaluated first:
// evaluates as 1 + Int("2") --> 3, which is Num, so "not Num" is false
print  1 + Int("2") is not Num // [expect] false