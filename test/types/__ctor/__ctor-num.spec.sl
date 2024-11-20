cls Foo{}
fn bar -> 1

print try Num()                           else error // [Expect] Expected 1 argument but got 0.
print try Num("123.0")                    else error // [Expect] Cannot instantiate a number via Num.ctor.
print try Num("1kjhkjh2hkjhkj.....3hkj4") else error // [Expect] Cannot instantiate a number via Num.ctor.
print try Num("false")                    else error // [Expect] Cannot instantiate a number via Num.ctor.
print try Num("true")                     else error // [Expect] Cannot instantiate a number via Num.ctor.
print try Num(true)                       else error // [Expect] Cannot instantiate a number via Num.ctor.
print try Num(false)                      else error // [Expect] Cannot instantiate a number via Num.ctor.
print try Num(12)                         else error // [Expect] Cannot instantiate a number via Num.ctor.
print try Num(12.0)                       else error // [Expect] Cannot instantiate a number via Num.ctor.
print try Num(123.456)                    else error // [Expect] Cannot instantiate a number via Num.ctor.
print try Num(nil)                        else error // [Expect] Cannot instantiate a number via Num.ctor.
print try Num([1, 2, 3])                  else error // [Expect] Cannot instantiate a number via Num.ctor.
print try Num((4, 5, 6))                  else error // [Expect] Cannot instantiate a number via Num.ctor.
print try Num({"key": "value"})           else error // [Expect] Cannot instantiate a number via Num.ctor.
print try Num(Foo)                        else error // [Expect] Cannot instantiate a number via Num.ctor.
print try Num(Foo())                      else error // [Expect] Cannot instantiate a number via Num.ctor.
print try Num(bar)                        else error // [Expect] Cannot instantiate a number via Num.ctor.

print try Int()                           else error // [Expect] Expected 1 argument but got 0.
print try Int("123.0")                    else error // [Expect] 123
print try Int("1kjhkjh2hkjhkj.....3hkj4") else error // [Expect] 12
print try Int("false")                    else error // [Expect] 0
print try Int("true")                     else error // [Expect] 0
print try Int(true)                       else error // [Expect] 1
print try Int(false)                      else error // [Expect] 0
print try Int(12)                         else error // [Expect] 12
print try Int(12.0)                       else error // [Expect] 12
print try Int(123.456)                    else error // [Expect] 123
print try Int(nil)                        else error // [Expect] 0
print try Int([1, 2, 3])                  else error // [Expect] 0
print try Int((4, 5, 6))                  else error // [Expect] 0
print try Int({"key": "value"})           else error // [Expect] 0
print try Int(Foo)                        else error // [Expect] 0
print try Int(Foo())                      else error // [Expect] 0
print try Int(bar)                        else error // [Expect] 0

print try Float()                           else error // [Expect] Expected 1 argument but got 0.
print try Float("123.0")                    else error // [Expect] 123
print try Float("1kjhkjh2hkjhkj.....3hkj4") else error // [Expect] 12.34
print try Float("false")                    else error // [Expect] 0
print try Float("true")                     else error // [Expect] 0
print try Float(true)                       else error // [Expect] 1
print try Float(false)                      else error // [Expect] 0
print try Float(12)                         else error // [Expect] 12
print try Float(12.0)                       else error // [Expect] 12
print try Float(123.456)                    else error // [Expect] 123.456
print try Float(nil)                        else error // [Expect] 0
print try Float([1, 2, 3])                  else error // [Expect] 0
print try Float((4, 5, 6))                  else error // [Expect] 0
print try Float({"key": "value"})           else error // [Expect] 0
print try Float(Foo)                        else error // [Expect] 0
print try Float(Foo())                      else error // [Expect] 0
print try Float(bar)                        else error // [Expect] 0