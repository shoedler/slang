print type_name(nil)           // [Expect] Nil

print type_name(false)         // [Expect] Bool
print type_name(true)          // [Expect] Bool

print type_name(0)             // [Expect] Num
print type_name(0.0)           // [Expect] Num
print type_name(-1)            // [Expect] Num
print type_name(-1.0)          // [Expect] Num

print type_name("")            // [Expect] Str
print type_name("hello")       // [Expect] Str

print type_name([])            // [Expect] Seq
print type_name([1, 2, 3])     // [Expect] Seq

print type_name(fn -> nil)     // [Expect] Fn
print type_name(fn(x) -> x)    // [Expect] Fn

print type_name(clock)         // [Expect] NativeFn
print type_name(type_name)     // [Expect] NativeFn

cls X { ctor {} fn y {}}
print type_name(Str)           // [Expect] Class
print type_name(Num)           // [Expect] Class
print type_name(X)             // [Expect] Class

let x = X()
print type_name(x)             // [Expect] Instance

print type_name(X.__ctor)      // [Expect] Fn
print type_name(0.to_str)      // [Expect] BoundMethod
print type_name(x.y)           // [Expect] BoundMethod