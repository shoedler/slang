cls Foo{}
fn bar -> 1

print try Bool()                           else error // [expect] Expected 1 argument but got 0.
print try Bool("123.0")                    else error // [expect] true
print try Bool("1kjhkjh2hkjhkj.....3hkj4") else error // [expect] true
print try Bool("false")                    else error // [expect] true
print try Bool("true")                     else error // [expect] true
print try Bool(true)                       else error // [expect] true
print try Bool(false)                      else error // [expect] false
print try Bool(12)                         else error // [expect] true
print try Bool(12.0)                       else error // [expect] true
print try Bool(123.456)                    else error // [expect] true
print try Bool(nil)                        else error // [expect] false
print try Bool([1, 2, 3])                  else error // [expect] true
print try Bool((4, 5, 6))                  else error // [expect] true
print try Bool({"key": "value"})           else error // [expect] true
print try Bool(Foo)                        else error // [expect] true
print try Bool(Foo())                      else error // [expect] true
print try Bool(bar)                        else error // [expect] true