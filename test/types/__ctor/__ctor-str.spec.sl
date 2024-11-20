cls Foo{}
fn bar -> 1

print try Str()                           else error // [Expect] Expected 1 argument but got 0.
print try Str("123.0")                    else error // [Expect] 123.0
print try Str("1kjhkjh2hkjhkj.....3hkj4") else error // [Expect] 1kjhkjh2hkjhkj.....3hkj4
print try Str("false")                    else error // [Expect] false
print try Str("true")                     else error // [Expect] true
print try Str(true)                       else error // [Expect] true
print try Str(false)                      else error // [Expect] false
print try Str(12)                         else error // [Expect] 12
print try Str(12.0)                       else error // [Expect] 12
print try Str(123.456)                    else error // [Expect] 123.456
print try Str(nil)                        else error // [Expect] nil
print try Str([1, 2, 3])                  else error // [Expect] [1, 2, 3]
print try Str((4, 5, 6))                  else error // [Expect] (4, 5, 6)
print try Str({"key": "value"})           else error // [Expect] {key: value}
print try Str(Foo)                        else error // [Expect] <Class Foo>
print try Str(Foo())                      else error // [Expect] <Instance of Foo>
print try Str(bar)                        else error // [Expect] <Fn bar>