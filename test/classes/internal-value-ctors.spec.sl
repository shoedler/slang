// Number
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

// Seq
// Only num- or no-param invocations are allowed
// print Seq("123.0")                    // Error
// print Seq("1kjhkjh2hkjhkj.....3hkj4") // Error
// print Seq("false")                    // Error
// print Seq("true")                     // Error
// print Seq(true)                       // Error
// print Seq(false)                      // Error
print Seq(12)                         // [Expect] [nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil]
print Seq(12.0)                       // [Expect] [nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil]
print Seq(12.456)                     // [Expect] [nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil]
// Only num- or no-param invocations are allowed
// print Seq(nil)                        // Error
// print Seq([1, 2, 3])                  // Error
// print Seq({"key": "value"})           // Error

// Map
print Map()                           // [Expect] {}
// Only no-param invocations are allowed
// print Map("123.0")                    // Error
// print Map("1kjhkjh2hkjhkj.....3hkj4") // Error
// print Map("false")                    // Error
// print Map("true")                     // Error
// print Map(true)                       // Error
// print Map(false)                      // Error
// print Map(12)                         // Error
// print Map(12.0)                       // Error
// print Map(12.456)                     // Error
// print Map(nil)                        // Error
// print Map([1, 2, 3])                  // Error
// print Map({"key": "value"})           // Error

