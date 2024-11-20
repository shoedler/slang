print typeof(nil)           // [Expect] <Class Nil>

print typeof(false)         // [Expect] <Class Bool>
print typeof(true)          // [Expect] <Class Bool>

print typeof(0)             // [Expect] <Class Int>
print typeof(0.0)           // [Expect] <Class Float>
print typeof(-1)            // [Expect] <Class Int>
print typeof(-1.0)          // [Expect] <Class Float>

print typeof("")            // [Expect] <Class Str>
print typeof("hello")       // [Expect] <Class Str>

print typeof([])            // [Expect] <Class Seq>
print typeof([1, 2, 3])     // [Expect] <Class Seq>

print typeof(fn -> nil)     // [Expect] <Class Fn>
print typeof(fn(x) -> x)    // [Expect] <Class Fn>

print typeof(clock)         // [Expect] <Class Fn>
print typeof(typeof)        // [Expect] <Class Fn>

cls X { ctor {} fn y {}}
print typeof(Str)           // [Expect] <Class Class>
print typeof(Num)           // [Expect] <Class Class>
print typeof(X)             // [Expect] <Class Class>

let x = X()
print typeof(x)             // [Expect] <Class X>

print typeof(X.ctor)        // [Expect] <Class Fn>
print typeof(0.to_str)      // [Expect] <Class Fn>
print typeof(x.y)           // [Expect] <Class Fn>