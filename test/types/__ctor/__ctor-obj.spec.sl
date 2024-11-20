cls Foo{}
fn bar -> 1

print try Obj()                           else error // [Expect] {}
print try Obj("123.0")                    else error // [Expect] Expected 0 arguments but got 1.
print try Obj("1kjhkjh2hkjhkj.....3hkj4") else error // [Expect] Expected 0 arguments but got 1.
print try Obj("false")                    else error // [Expect] Expected 0 arguments but got 1.
print try Obj("true")                     else error // [Expect] Expected 0 arguments but got 1.
print try Obj(true)                       else error // [Expect] Expected 0 arguments but got 1.
print try Obj(false)                      else error // [Expect] Expected 0 arguments but got 1.
print try Obj(12)                         else error // [Expect] Expected 0 arguments but got 1.
print try Obj(12.0)                       else error // [Expect] Expected 0 arguments but got 1.
print try Obj(123.456)                    else error // [Expect] Expected 0 arguments but got 1.
print try Obj(nil)                        else error // [Expect] Expected 0 arguments but got 1.
print try Obj([1, 2, 3])                  else error // [Expect] Expected 0 arguments but got 1.
print try Obj((4, 5, 6))                  else error // [Expect] Expected 0 arguments but got 1.
print try Obj({"key": "value"})           else error // [Expect] Expected 0 arguments but got 1.
print try Obj(Foo)                        else error // [Expect] Expected 0 arguments but got 1.
print try Obj(Foo())                      else error // [Expect] Expected 0 arguments but got 1.
print try Obj(bar)                        else error // [Expect] Expected 0 arguments but got 1.