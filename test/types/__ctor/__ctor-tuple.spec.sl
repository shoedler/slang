cls Foo{}
fn bar -> 1

print try Tuple()                           else error // [expect] Expected 1 argument but got 0.
print try Tuple("123.0")                    else error // [expect] Expected argument 0 of type Seq but got Str.
print try Tuple("1kjhkjh2hkjhkj.....3hkj4") else error // [expect] Expected argument 0 of type Seq but got Str.
print try Tuple("false")                    else error // [expect] Expected argument 0 of type Seq but got Str.
print try Tuple("true")                     else error // [expect] Expected argument 0 of type Seq but got Str.
print try Tuple(true)                       else error // [expect] Expected argument 0 of type Seq but got Bool.
print try Tuple(false)                      else error // [expect] Expected argument 0 of type Seq but got Bool.
print try Tuple(12)                         else error // [expect] Expected argument 0 of type Seq but got Int.
print try Tuple(12.0)                       else error // [expect] Expected argument 0 of type Seq but got Float.
print try Tuple(123.456)                    else error // [expect] Expected argument 0 of type Seq but got Float.
print try Tuple(nil)                        else error // [expect] Expected argument 0 of type Seq but got Nil.
print try Tuple([1, 2, 3])                  else error // [expect] (1, 2, 3)
print try Tuple((4, 5, 6))                  else error // [expect] Expected argument 0 of type Seq but got Tuple.
print try Tuple({"key": "value"})           else error // [expect] Expected argument 0 of type Seq but got Obj.
print try Tuple(Foo)                        else error // [expect] Expected argument 0 of type Seq but got Class.
print try Tuple(Foo())                      else error // [expect] Expected argument 0 of type Seq but got Foo.
print try Tuple(bar)                        else error // [expect] Expected argument 0 of type Seq but got Fn.

// Sanity check for Tuple(Seq) ctor to make sure internally they don't use the same value array.
let a = [1, 2, 3]
let b = Tuple(a)
a.push(4)
print b // [expect] (1, 2, 3)
print a // [expect] [1, 2, 3, 4]