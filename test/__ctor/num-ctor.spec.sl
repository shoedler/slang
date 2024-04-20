cls Foo{}
fn bar -> 1

print try Num()                           else error // [Expect] Expected 1 arguments but got 0.
print try Num("123.0")                    else error // [Expect] 123
print try Num("1kjhkjh2hkjhkj.....3hkj4") else error // [Expect] 12.34
print try Num("false")                    else error // [Expect] 0
print try Num("true")                     else error // [Expect] 0
print try Num(true)                       else error // [Expect] 1
print try Num(false)                      else error // [Expect] 0
print try Num(12)                         else error // [Expect] 12
print try Num(12.0)                       else error // [Expect] 12
print try Num(123.456)                    else error // [Expect] 123.456
print try Num(nil)                        else error // [Expect] 0
print try Num([1, 2, 3])                  else error // [Expect] 0
print try Num({"key": "value"})           else error // [Expect] 0
print try Num(Foo)                        else error // [Expect] 0
print try Num(Foo())                      else error // [Expect] 0
print try Num(bar)                        else error // [Expect] 0
