cls Foo{}
fn bar -> 1

print try Class()                           else error // [Expect] Cannot instantiate a class via Class.ctor.
print try Class("123.0")                    else error // [Expect] Cannot instantiate a class via Class.ctor.
print try Class("1kjhkjh2hkjhkj.....3hkj4") else error // [Expect] Cannot instantiate a class via Class.ctor.
print try Class("false")                    else error // [Expect] Cannot instantiate a class via Class.ctor.
print try Class("true")                     else error // [Expect] Cannot instantiate a class via Class.ctor.
print try Class(true)                       else error // [Expect] Cannot instantiate a class via Class.ctor.
print try Class(false)                      else error // [Expect] Cannot instantiate a class via Class.ctor.
print try Class(12)                         else error // [Expect] Cannot instantiate a class via Class.ctor.
print try Class(12.0)                       else error // [Expect] Cannot instantiate a class via Class.ctor.
print try Class(123.456)                    else error // [Expect] Cannot instantiate a class via Class.ctor.
print try Class(nil)                        else error // [Expect] Cannot instantiate a class via Class.ctor.
print try Class([1, 2, 3])                  else error // [Expect] Cannot instantiate a class via Class.ctor.
print try Class({"key": "value"})           else error // [Expect] Cannot instantiate a class via Class.ctor.
print try Class(Foo)                        else error // [Expect] Cannot instantiate a class via Class.ctor.
print try Class(Foo())                      else error // [Expect] Cannot instantiate a class via Class.ctor.
print try Class(bar)                        else error // [Expect] Cannot instantiate a class via Class.ctor.