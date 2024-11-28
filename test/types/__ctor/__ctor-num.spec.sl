cls Foo{}
fn bar -> 1

print try Num()                           else error // [expect] Expected 1 argument but got 0.
print try Num("123.0")                    else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num("1kjhkjh2hkjhkj.....3hkj4") else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num("0x")                       else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num("0x123")                    else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num("false")                    else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num("true")                     else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num(true)                       else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num(false)                      else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num(12)                         else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num(12.0)                       else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num(123.456)                    else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num(nil)                        else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num([1, 2, 3])                  else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num((4, 5, 6))                  else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num({"key": "value"})           else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num(Foo)                        else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num(Foo())                      else error // [expect] Cannot instantiate a number via Num.ctor.
print try Num(bar)                        else error // [expect] Cannot instantiate a number via Num.ctor.
print try (Num("123.1") is Num)           else error // [expect] Cannot instantiate a number via Num.ctor.

print try Int()                           else error // [expect] Expected 1 argument but got 0.
print try Int("123.0")                    else error // [expect] 123
print try Int("1kjhkjh2hkjhkj.....3hkj4") else error // [expect] 1
print try Int("0x")                       else error // [expect] 0
print try Int("0x123")                    else error // [expect] 291
print try Int("false")                    else error // [expect] 0
print try Int("true")                     else error // [expect] 0
print try Int(true)                       else error // [expect] 1
print try Int(false)                      else error // [expect] 0
print try Int(12)                         else error // [expect] 12
print try Int(12.0)                       else error // [expect] 12
print try Int(123.456)                    else error // [expect] 123
print try Int(nil)                        else error // [expect] 0
print try Int([1, 2, 3])                  else error // [expect] 0
print try Int((4, 5, 6))                  else error // [expect] 0
print try Int({"key": "value"})           else error // [expect] 0
print try Int(Foo)                        else error // [expect] 0
print try Int(Foo())                      else error // [expect] 0
print try Int(bar)                        else error // [expect] 0
print try (Int("123.1") is Int)           else error // [expect] true

print try Float()                           else error // [expect] Expected 1 argument but got 0.
print try Float("123.0")                    else error // [expect] 123
print try Float("1kjhkjh2hkjhkj.....3hkj4") else error // [expect] 1
print try Float("0x")                       else error // [expect] 0
print try Float("0x123")                    else error // [expect] 291
print try Float("false")                    else error // [expect] 0
print try Float("true")                     else error // [expect] 0
print try Float(true)                       else error // [expect] 1
print try Float(false)                      else error // [expect] 0
print try Float(12)                         else error // [expect] 12
print try Float(12.0)                       else error // [expect] 12
print try Float(123.456)                    else error // [expect] 123.456
print try Float(nil)                        else error // [expect] 0
print try Float([1, 2, 3])                  else error // [expect] 0
print try Float((4, 5, 6))                  else error // [expect] 0
print try Float({"key": "value"})           else error // [expect] 0
print try Float(Foo)                        else error // [expect] 0
print try Float(Foo())                      else error // [expect] 0
print try Float(bar)                        else error // [expect] 0
print try (Float("123") is Float)           else error // [expect] true

// Fuzzy
print Int("    123") // [expect] 123
print Int("123    ") // [expect] 123
print typeof(Float("0xDEAD")) // [expect] <Float>

// Float/Int comparison should work
print Float("0xDEAD") == Int("0xDEAD") // [expect] true
print Float("12.3") == Int("12.3")     // [expect] false
