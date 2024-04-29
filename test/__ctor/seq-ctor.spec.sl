cls Foo{}
fn bar -> 1

print try Seq()                           else error // [Expect] Expected 1 arguments but got 0.
print try Seq("123.0")                    else error // [Expect] Expected argument 0 of type Int or Tuple but got Str.
print try Seq("1kjhkjh2hkjhkj.....3hkj4") else error // [Expect] Expected argument 0 of type Int or Tuple but got Str.
print try Seq("false")                    else error // [Expect] Expected argument 0 of type Int or Tuple but got Str.
print try Seq("true")                     else error // [Expect] Expected argument 0 of type Int or Tuple but got Str.
print try Seq(true)                       else error // [Expect] Expected argument 0 of type Int or Tuple but got Bool.
print try Seq(false)                      else error // [Expect] Expected argument 0 of type Int or Tuple but got Bool.
print try Seq(12)                         else error // [Expect] [nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil, nil]
print try Seq(12.0)                       else error // [Expect] Expected argument 0 of type Int or Tuple but got Float.
print try Seq(123.456)                    else error // [Expect] Expected argument 0 of type Int or Tuple but got Float.
print try Seq(nil)                        else error // [Expect] Expected argument 0 of type Int or Tuple but got Nil.
print try Seq([1, 2, 3])                  else error // [Expect] Expected argument 0 of type Int or Tuple but got Seq.
print try Seq((4, 5, 6))                  else error // [Expect] [4, 5, 6]
print try Seq({"key": "value"})           else error // [Expect] Expected argument 0 of type Int or Tuple but got Obj.
print try Seq(Foo)                        else error // [Expect] Expected argument 0 of type Int or Tuple but got Class.
print try Seq(Foo())                      else error // [Expect] Expected argument 0 of type Int or Tuple but got Foo.
print try Seq(bar)                        else error // [Expect] Expected argument 0 of type Int or Tuple but got Fn.

// Sanity check for Seq(Tuple) overload to make sure internally they don't use the same value array.
let a = (1, 2, 3)
let b = Seq(a)
b.push(4)
print b // [Expect] [1, 2, 3, 4]
print a // [Expect] (1, 2, 3)