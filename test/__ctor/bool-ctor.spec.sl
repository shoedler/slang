cls Foo{}
fn bar -> 1

print try Bool()                           else error // [Expect] Expected 1 arguments but got 0.
print try Bool("123.0")                    else error // [Expect] true
print try Bool("1kjhkjh2hkjhkj.....3hkj4") else error // [Expect] true
print try Bool("false")                    else error // [Expect] true
print try Bool("true")                     else error // [Expect] true
print try Bool(true)                       else error // [Expect] true
print try Bool(false)                      else error // [Expect] false
print try Bool(12)                         else error // [Expect] true
print try Bool(12.0)                       else error // [Expect] true
print try Bool(123.456)                    else error // [Expect] true
print try Bool(nil)                        else error // [Expect] false
print try Bool([1, 2, 3])                  else error // [Expect] true
print try Bool((4, 5, 6))                  else error // [Expect] true
print try Bool({"key": "value"})           else error // [Expect] true
print try Bool(Foo)                        else error // [Expect] true
print try Bool(Foo())                      else error // [Expect] true
print try Bool(bar)                        else error // [Expect] true