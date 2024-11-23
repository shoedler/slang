cls Foo{}
fn bar -> 1

print try Str()                           else error // [expect] Expected 1 argument but got 0.
print try Str("123.0")                    else error // [expect] 123.0
print try Str("1kjhkjh2hkjhkj.....3hkj4") else error // [expect] 1kjhkjh2hkjhkj.....3hkj4
print try Str("false")                    else error // [expect] false
print try Str("true")                     else error // [expect] true
print try Str(true)                       else error // [expect] true
print try Str(false)                      else error // [expect] false
print try Str(12)                         else error // [expect] 12
print try Str(12.0)                       else error // [expect] 12
print try Str(123.456)                    else error // [expect] 123.456
print try Str(nil)                        else error // [expect] nil
print try Str([1, 2, 3])                  else error // [expect] [1, 2, 3]
print try Str((4, 5, 6))                  else error // [expect] (4, 5, 6)
print try Str({"key": "value"})           else error // [expect] {key: value}
print try Str(Foo)                        else error // [expect] <Foo>
print try Str(Foo())                      else error // [expect] <Instance of Foo>
print try Str(bar)                        else error // [expect] <Fn bar>