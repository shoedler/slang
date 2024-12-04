cls X{}
cls Y{ fn mul(x) -> 42 }
cls Z{ fn mul(x) -> ["Lol"] } // Doesn't have to return a Num

print try nil * 2 else error        // [expect] Type Nil does not support "mul".
print try true * 2 else error       // [expect] Type Bool does not support "mul".
print try Int(1) * 2 else error     // [expect] 2
print try Float(1.0) * 2 else error // [expect] 2
print try "1" * 2 else error        // [expect] Type Str does not support "mul".
print try [1] * 2 else error        // [expect] Type Seq does not support "mul".
print try (1,) * 2 else error       // [expect] Type Tuple does not support "mul".
print try {} * 2 else error         // [expect] Type Obj does not support "mul".
print try (fn -> 1) * 2 else error  // [expect] Type Fn does not support "mul".
print try X * 2 else error          // [expect] Type Class does not support "mul".
print try X() * 2 else error        // [expect] Type X does not support "mul".
print try Y() * 2 else error        // [expect] 42
print try Z() * 2 else error        // [expect] [Lol]

print 1 * 0    // [expect] 0
print 1 * 1    // [expect] 1
print 1 * 2    // [expect] 2
print -1 * -0  // [expect] 0
print -0 * 0   // [expect] 0
print -1 * -2  // [expect] 2
print -2 * -1  // [expect] 2
print 123 * 20 // [expect] 2460
print 123 * 5  // [expect] 615
