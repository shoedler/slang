print type_of(nil)           // [Expect] <Class Nil>

print type_of(false)         // [Expect] <Class Bool>
print type_of(true)          // [Expect] <Class Bool>

print type_of(0)             // [Expect] <Class Num>
print type_of(0.0)           // [Expect] <Class Num>
print type_of(-1)            // [Expect] <Class Num>
print type_of(-1.0)          // [Expect] <Class Num>

print type_of("")            // [Expect] <Class Str>
print type_of("hello")       // [Expect] <Class Str>

print type_of([])            // [Expect] <Class Seq>
print type_of([1, 2, 3])     // [Expect] <Class Seq>

print type_of(fn -> nil)     // [Expect] <Class Obj>
print type_of(fn(x) -> x)    // [Expect] <Class Obj>

print type_of(clock)         // [Expect] <Class Obj>
print type_of(type_of)       // [Expect] <Class Obj>

cls X { ctor {} fn y {}}
print type_of(Str)           // [Expect] <Class Obj>
print type_of(Num)           // [Expect] <Class Obj>
print type_of(X)             // [Expect] <Class Obj>

let x = X()
print type_of(x)             // [Expect] <Class X>

print type_of(X.__ctor)      // [Expect] <Class Obj>
print type_of(0.to_str)      // [Expect] <Class Obj>
print type_of(x.y)           // [Expect] <Class Obj>