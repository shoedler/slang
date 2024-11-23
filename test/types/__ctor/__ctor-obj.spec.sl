cls Foo{}
fn bar -> 1

print try Obj()                           else error // [expect] {}
print try Obj("123.0")                    else error // [expect] Expected 0 arguments but got 1.
print try Obj("1kjhkjh2hkjhkj.....3hkj4") else error // [expect] Expected 0 arguments but got 1.
print try Obj("false")                    else error // [expect] Expected 0 arguments but got 1.
print try Obj("true")                     else error // [expect] Expected 0 arguments but got 1.
print try Obj(true)                       else error // [expect] Expected 0 arguments but got 1.
print try Obj(false)                      else error // [expect] Expected 0 arguments but got 1.
print try Obj(12)                         else error // [expect] Expected 0 arguments but got 1.
print try Obj(12.0)                       else error // [expect] Expected 0 arguments but got 1.
print try Obj(123.456)                    else error // [expect] Expected 0 arguments but got 1.
print try Obj(nil)                        else error // [expect] Expected 0 arguments but got 1.
print try Obj([1, 2, 3])                  else error // [expect] Expected 0 arguments but got 1.
print try Obj((4, 5, 6))                  else error // [expect] Expected 0 arguments but got 1.
print try Obj({"key": "value"})           else error // [expect] Expected 0 arguments but got 1.
print try Obj(Foo)                        else error // [expect] Expected 0 arguments but got 1.
print try Obj(Foo())                      else error // [expect] Expected 0 arguments but got 1.
print try Obj(bar)                        else error // [expect] Expected 0 arguments but got 1.