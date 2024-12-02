cls X{}
cls Y{ fn add(x) -> 42 }
cls Z{ fn add(x) -> ["Lol"] } // Doesn't have to return a Num

print try nil + 2 else error        // [expect] Type Nil does not support the '+' operator. It must implement 'add'.
print try true + 2 else error       // [expect] Type Bool does not support the '+' operator. It must implement 'add'.
print try Int(1) + 2 else error     // [expect] 3
print try Float(1.0) + 2 else error // [expect] 3
print try "1" + 2 else error        // [expect] 12
print try [1] + 2 else error        // [expect] Type Seq does not support the '+' operator. It must implement 'add'.
print try (1,) + 2 else error       // [expect] Type Tuple does not support the '+' operator. It must implement 'add'.
print try {} + 2 else error         // [expect] Type Obj does not support the '+' operator. It must implement 'add'.
print try (fn -> 1) + 2 else error  // [expect] Type Fn does not support the '+' operator. It must implement 'add'.
print try X + 2 else error          // [expect] Type Class does not support the '+' operator. It must implement 'add'.
print try X() + 2 else error        // [expect] Type X does not support the '+' operator. It must implement 'add'.
print try Y() + 2 else error        // [expect] 42
print try Z() + 2 else error        // [expect] [Lol]

print 1 + 0    // [expect] 1
print 1 + 1    // [expect] 2
print 1 + 2    // [expect] 3
print -1 + -0  // [expect] -1
print -0 + 0   // [expect] 0
print -1 + -2  // [expect] -3
print -2 + -1  // [expect] -3
print 123 + 20 // [expect] 143
print 123 + 5  // [expect] 128

// String concatenation
print "Lo" + "l"         // [expect] Lol
print "true" + true      // [expect] truetrue
print "1" + 1            // [expect] 11
print "1" + 1.0          // [expect] 11
print "Lol" + [1,2,3]    // [expect] Lol[1, 2, 3]
print "Lol" + (1,2,3)    // [expect] Lol(1, 2, 3)
print "Lol" + {"a":"b"}  // [expect] Lol{a: b}
print "Lol" + (fn -> 1)  // [expect] Lol<Fn (anon)>
print "Lol" + X          // [expect] Lol<X>
print "Lol" + X()        // [expect] Lol<Instance of X> 
print "Lol" + Y()        // [expect] Lol<Instance of Y>
print "Lol" + Z()        // [expect] Lol<Instance of Z>

