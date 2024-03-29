// Number
print Num()                           // [Expect] 0
print Num("123.0")                    // [Expect] 123
print Num("1kjhkjh2hkjhkj.....3hkj4") // [Expect] 12.340000
print Num("false")                    // [Expect] 0
print Num("true")                     // [Expect] 0
print Num(true)                       // [Expect] 1
print Num(false)                      // [Expect] 0
print Num(123)                        // [Expect] 123
print Num(123.0)                      // [Expect] 123
print Num(123.456)                    // [Expect] 123.456000
print Num(nil)                        // [Expect] 0
print Num([1, 2, 3])                  // [Expect] 0
print Num({"key": "value"})           // [Expect] 0

// Bool
print Bool()                           // [Expect] false
print Bool("123.0")                    // [Expect] true
print Bool("1kjhkjh2hkjhkj.....3hkj4") // [Expect] true
print Bool("false")                    // [Expect] true
print Bool("true")                     // [Expect] true
print Bool(true)                       // [Expect] true
print Bool(false)                      // [Expect] false
print Bool(123)                        // [Expect] true
print Bool(123.0)                      // [Expect] true
print Bool(123.456)                    // [Expect] true
print Bool(nil)                        // [Expect] false
print Bool([1, 2, 3])                  // [Expect] true
print Bool({"key": "value"})           // [Expect] true

// Str
// Empty Str ctor returns "" which we can't "expect" - I'm sure it works - lol
// print Str()                           // //Expect 
print Str("123.0")                    // [Expect] 123.0
print Str("1kjhkjh2hkjhkj.....3hkj4") // [Expect] 1kjhkjh2hkjhkj.....3hkj4
print Str("false")                    // [Expect] false
print Str("true")                     // [Expect] true
print Str(true)                       // [Expect] true
print Str(false)                      // [Expect] false
print Str(123)                        // [Expect] 123
print Str(123.0)                      // [Expect] 123
print Str(123.456)                    // [Expect] 123.456000
print Str(nil)                        // [Expect] nil
print Str([1, 2, 3])                  // [Expect] [1, 2, 3]
// Order of keys in map is not guaranteed
// print Str({"key": "value"})           // //Expect {"key": "value"}

// Nil
print Nil()                           // [Expect] nil
print Nil("123.0")                    // [Expect] nil
print Nil("1kjhkjh2hkjhkj.....3hkj4") // [Expect] nil
print Nil("false")                    // [Expect] nil
print Nil("true")                     // [Expect] nil
print Nil(true)                       // [Expect] nil
print Nil(false)                      // [Expect] nil
print Nil(123)                        // [Expect] nil
print Nil(123.0)                      // [Expect] nil
print Nil(123.456)                    // [Expect] nil
print Nil(nil)                        // [Expect] nil
print Nil([1, 2, 3])                  // [Expect] nil
print Nil({"key": "value"})           // [Expect] nil

// Seq
print Seq()                           // [Expect] []
// Only num- or no-param invocations are allowed
// print Seq("123.0")                    // //Expect []
// print Seq("1kjhkjh2hkjhkj.....3hkj4") // //Expect []
// print Seq("false")                    // //Expect []
// print Seq("true")                     // //Expect []
// print Seq(true)                       // //Expect []
// print Seq(false)                      // //Expect []
print Seq(12)                         // [Expect] [nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil]
print Seq(12.0)                       // [Expect] [nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil]
print Seq(12.456)                     // [Expect] [nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil]
// Only num- or no-param invocations are allowed
// print Seq(nil)                        // //Expect []
// print Seq([1, 2, 3])                  // //Expect []
// print Seq({"key": "value"})           // //Expect []

// Map
print Map()                           // [Expect] {}
// Only no-param invocations are allowed
// print Map("123.0")                    // //Expect {}
// print Map("1kjhkjh2hkjhkj.....3hkj4") // //Expect {}
// print Map("false")                    // //Expect {}
// print Map("true")                     // //Expect {}
// print Map(true)                       // //Expect {}
// print Map(false)                      // //Expect {}
// print Map(12)                         // //Expect {}
// print Map(12.0)                       // //Expect {}
// print Map(12.456)                     // //Expect {}
// print Map(nil)                        // //Expect {}
// print Map([1, 2, 3])                  // //Expect {}
// print Map({"key": "value"})           // //Expect {}

