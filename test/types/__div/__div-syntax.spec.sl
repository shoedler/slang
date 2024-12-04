cls X{}
cls Y{ fn div(x) -> 42 }
cls Z{ fn div(x) -> ["Lol"] } // Doesn't have to return a Num

print try nil / 2 else error        // [expect] Type Nil does not support "div".
print try true / 2 else error       // [expect] Type Bool does not support "div".
print try Int(1) / 2 else error     // [expect] 0.5
print try Float(1.0) / 2 else error // [expect] 0.5
print try "1" / 2 else error        // [expect] Type Str does not support "div".
print try [1] / 2 else error        // [expect] Type Seq does not support "div".
print try (1,) / 2 else error       // [expect] Type Tuple does not support "div".
print try {} / 2 else error         // [expect] Type Obj does not support "div".
print try (fn -> 1) / 2 else error  // [expect] Type Fn does not support "div".
print try X / 2 else error          // [expect] Type Class does not support "div".
print try X() / 2 else error        // [expect] Type X does not support "div".
print try Y() / 2 else error        // [expect] 42
print try Z() / 2 else error        // [expect] [Lol]

print try 1 / 0 else error   // [expect] Division by zero.
print 1 / 1                  // [expect] 1
print 1 / 2                  // [expect] 0.5
print try -1 / -0 else error // [expect] Division by zero.
print try -0 / 0 else error  // [expect] Division by zero.
print -1 / -2                // [expect] 0.5
print -2 / -1                // [expect] 2
print 123 / 20               // [expect] 6.15
print 123 / 5                // [expect] 24.6
