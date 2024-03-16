print Num()                           // [Expect] 0
print Num("123.0")                    // [Expect] 123
print Num("1kjhkjh2hkjhkj.....3hkj4") // [Expect] 12.340000
print Num(true)                       // [Expect] 1
print Num(false)                      // [Expect] 0
print Num(123)                        // [Expect] 123
print Num(nil)                        // [Expect] 0

print Bool()                          // [Expect] false
print Bool(false)                     // [Expect] false
print Bool(nil)                       // [Expect] false
print Bool(true)                      // [Expect] true
print Bool(0)                         // [Expect] true
print Bool("false")                   // [Expect] true

// Str() returns "" which we can't "expect" - I'm sure it works - lol
print Str(10)                         // [Expect] 10
print Str(true)                       // [Expect] true
print Str(false)                      // [Expect] false
print Str(nil)                        // [Expect] nil
print Str([1, 2, 3])                  // [Expect] [1, 2, 3]

print Nil()        // [Expect] nil
print Nil(true)    // [Expect] nil
print Nil(false)   // [Expect] nil
print Nil(0)       // [Expect] nil
print Nil("false") // [Expect] nil

print Seq()    // [Expect] []
print Seq(10)  // [Expect] [nil, nil, nil, nil, nil, nil, nil, nil, nil, nil]
