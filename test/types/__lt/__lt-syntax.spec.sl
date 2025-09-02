cls X{}
cls Y{ fn lt(x) -> true }

print try nil < 2 else error        // [expect] Type Nil does not support "lt".
print try true < 2 else error       // [expect] Type Bool does not support "lt".
print try Int(1) < 2 else error     // [expect] true
print try Float(1.0) < 2 else error // [expect] true
print try "1" < "2" else error      // [expect] true
print try [1] < 2 else error        // [expect] Type Seq does not support "lt".
print try (1,) < 2 else error       // [expect] Type Tuple does not support "lt".
print try {} < 2 else error         // [expect] Type Obj does not support "lt".
print try (fn -> 1) < 2 else error  // [expect] Type Fn does not support "lt".
print try X < 2 else error          // [expect] Type Class does not support "lt".
print try X() < 2 else error        // [expect] Type X does not support "lt".
print try Y() < 2 else error        // [expect] true

print 1 < 0   // [expect] false
print 1 < 1   // [expect] false
print 1 < 2   // [expect] true
print -1 < -0 // [expect] true
print -0 < 0  // [expect] false
print -1 < -2 // [expect] false
print -2 < -1 // [expect] true
