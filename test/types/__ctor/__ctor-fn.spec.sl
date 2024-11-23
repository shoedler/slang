cls Foo{}
fn bar -> 1

print try Fn()                           else error // [expect] Expected 1 argument but got 0.
print try Fn("123.0")                    else error // [expect] Cannot instantiate a function via Fn.ctor.
print try Fn("1kjhkjh2hkjhkj.....3hkj4") else error // [expect] Cannot instantiate a function via Fn.ctor.
print try Fn("false")                    else error // [expect] Cannot instantiate a function via Fn.ctor.
print try Fn("true")                     else error // [expect] Cannot instantiate a function via Fn.ctor.
print try Fn(true)                       else error // [expect] Cannot instantiate a function via Fn.ctor.
print try Fn(false)                      else error // [expect] Cannot instantiate a function via Fn.ctor.
print try Fn(12)                         else error // [expect] Cannot instantiate a function via Fn.ctor.
print try Fn(12.0)                       else error // [expect] Cannot instantiate a function via Fn.ctor.
print try Fn(123.456)                    else error // [expect] Cannot instantiate a function via Fn.ctor.
print try Fn(nil)                        else error // [expect] Cannot instantiate a function via Fn.ctor.
print try Fn([1, 2, 3])                  else error // [expect] Cannot instantiate a function via Fn.ctor.
print try Fn((4, 5, 6))                  else error // [expect] Cannot instantiate a function via Fn.ctor.
print try Fn({"key": "value"})           else error // [expect] Cannot instantiate a function via Fn.ctor.
print try Fn(Foo)                        else error // [expect] Cannot instantiate a function via Fn.ctor.
print try Fn(Foo())                      else error // [expect] Cannot instantiate a function via Fn.ctor.
print try Fn(bar)                        else error // [expect] Cannot instantiate a function via Fn.ctor.