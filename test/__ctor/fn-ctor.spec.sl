cls Foo{}
fn bar -> 1

print try Fn()                           else error // [Expect] Cannot instantiate a function via Fn.ctor.
print try Fn("123.0")                    else error // [Expect] Cannot instantiate a function via Fn.ctor.
print try Fn("1kjhkjh2hkjhkj.....3hkj4") else error // [Expect] Cannot instantiate a function via Fn.ctor.
print try Fn("false")                    else error // [Expect] Cannot instantiate a function via Fn.ctor.
print try Fn("true")                     else error // [Expect] Cannot instantiate a function via Fn.ctor.
print try Fn(true)                       else error // [Expect] Cannot instantiate a function via Fn.ctor.
print try Fn(false)                      else error // [Expect] Cannot instantiate a function via Fn.ctor.
print try Fn(12)                         else error // [Expect] Cannot instantiate a function via Fn.ctor.
print try Fn(12.0)                       else error // [Expect] Cannot instantiate a function via Fn.ctor.
print try Fn(123.456)                    else error // [Expect] Cannot instantiate a function via Fn.ctor.
print try Fn(nil)                        else error // [Expect] Cannot instantiate a function via Fn.ctor.
print try Fn([1, 2, 3])                  else error // [Expect] Cannot instantiate a function via Fn.ctor.
print try Fn({"key": "value"})           else error // [Expect] Cannot instantiate a function via Fn.ctor.
print try Fn(Foo)                        else error // [Expect] Cannot instantiate a function via Fn.ctor.
print try Fn(Foo())                      else error // [Expect] Cannot instantiate a function via Fn.ctor.
print try Fn(bar)                        else error // [Expect] Cannot instantiate a function via Fn.ctor.