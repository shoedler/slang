cls Foo{}
fn bar -> 1

print try Nil()                           else error // [Expect] Expected 1 argument but got 0.
print try Nil("123.0")                    else error // [Expect] Cannot instantiate nil via Nil.ctor.
print try Nil("1kjhkjh2hkjhkj.....3hkj4") else error // [Expect] Cannot instantiate nil via Nil.ctor.
print try Nil("false")                    else error // [Expect] Cannot instantiate nil via Nil.ctor.
print try Nil("true")                     else error // [Expect] Cannot instantiate nil via Nil.ctor.
print try Nil(true)                       else error // [Expect] Cannot instantiate nil via Nil.ctor.
print try Nil(false)                      else error // [Expect] Cannot instantiate nil via Nil.ctor.
print try Nil(12)                         else error // [Expect] Cannot instantiate nil via Nil.ctor.
print try Nil(12.0)                       else error // [Expect] Cannot instantiate nil via Nil.ctor.
print try Nil(123.456)                    else error // [Expect] Cannot instantiate nil via Nil.ctor.
print try Nil(nil)                        else error // [Expect] Cannot instantiate nil via Nil.ctor.
print try Nil([1, 2, 3])                  else error // [Expect] Cannot instantiate nil via Nil.ctor.
print try Nil((4, 5, 6))                  else error // [Expect] Cannot instantiate nil via Nil.ctor.
print try Nil({"key": "value"})           else error // [Expect] Cannot instantiate nil via Nil.ctor.
print try Nil(Foo)                        else error // [Expect] Cannot instantiate nil via Nil.ctor.
print try Nil(Foo())                      else error // [Expect] Cannot instantiate nil via Nil.ctor.
print try Nil(bar)                        else error // [Expect] Cannot instantiate nil via Nil.ctor.