print typeof(nil)           // [expect] <Nil>

print typeof(false)         // [expect] <Bool>
print typeof(true)          // [expect] <Bool>

print typeof(0)             // [expect] <Int>
print typeof(0.0)           // [expect] <Float>
print typeof(-1)            // [expect] <Int>
print typeof(-1.0)          // [expect] <Float>

print typeof("")            // [expect] <Str>
print typeof("hello")       // [expect] <Str>

print typeof([])            // [expect] <Seq>
print typeof([1, 2, 3])     // [expect] <Seq>

print typeof(fn -> nil)     // [expect] <Fn>
print typeof(fn(x) -> x)    // [expect] <Fn>

print typeof(clock)         // [expect] <Fn>
print typeof(typeof)        // [expect] <Fn>

cls X { ctor {} fn y {}}
print typeof(Str)           // [expect] <Class>
print typeof(Num)           // [expect] <Class>
print X                     // [expect] <X>
print typeof(X)             // [expect] <Class>

let x = X()
print x                     // [expect] <Instance of X>
print typeof(x)             // [expect] <X>

print typeof(X.ctor)        // [expect] <Fn>
print typeof(0.to_str)      // [expect] <Fn>
print typeof(x.y)           // [expect] <Fn>