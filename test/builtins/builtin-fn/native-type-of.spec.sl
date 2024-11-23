print typeof(nil)           // [expect] <Class Nil>

print typeof(false)         // [expect] <Class Bool>
print typeof(true)          // [expect] <Class Bool>

print typeof(0)             // [expect] <Class Int>
print typeof(0.0)           // [expect] <Class Float>
print typeof(-1)            // [expect] <Class Int>
print typeof(-1.0)          // [expect] <Class Float>

print typeof("")            // [expect] <Class Str>
print typeof("hello")       // [expect] <Class Str>

print typeof([])            // [expect] <Class Seq>
print typeof([1, 2, 3])     // [expect] <Class Seq>

print typeof(fn -> nil)     // [expect] <Class Fn>
print typeof(fn(x) -> x)    // [expect] <Class Fn>

print typeof(clock)         // [expect] <Class Fn>
print typeof(typeof)        // [expect] <Class Fn>

cls X { ctor {} fn y {}}
print typeof(Str)           // [expect] <Class Class>
print typeof(Num)           // [expect] <Class Class>
print typeof(X)             // [expect] <Class Class>

let x = X()
print typeof(x)             // [expect] <Class X>

print typeof(X.ctor)        // [expect] <Class Fn>
print typeof(0.to_str)      // [expect] <Class Fn>
print typeof(x.y)           // [expect] <Class Fn>