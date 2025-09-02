// is not Num
print 0 is not Num      // [expect] false
print 1.1 is not Num    // [expect] false
print 0b1 is not Num    // [expect] false
print 0o1 is not Num    // [expect] false
print 0x1 is not Num    // [expect] false
print "str" is not Num  // [expect] true

// is not Int
print 0 is not Int      // [expect] false
print 1.1 is not Int    // [expect] true
print 0x1 is not Int    // [expect] false
print 0b1 is not Int    // [expect] false
print 0o1 is not Int    // [expect] false

// is not Float
print 0 is not Float    // [expect] true
print 1.1 is not Float  // [expect] false
print 0x1 is not Float  // [expect] true
print 0b1 is not Float  // [expect] true
print 0o1 is not Float  // [expect] true

// is not Bool
print true is not Bool   // [expect] false
print false is not Bool  // [expect] false
print {} is not Bool     // [expect] true

// is not Nil
print nil is not Nil     // [expect] false
print true is not Nil    // [expect] true

// is not Seq
print [] is not Seq      // [expect] false
print [1, 2] is not Seq  // [expect] false
print {} is not Seq      // [expect] true

// is not Obj
print {} is not Obj      // [expect] false
print {1: 2} is not Obj  // [expect] false
print [] is not Obj      // [expect] true
print "" is not Obj      // [expect] true
// nil, num, bool do not inherit from Obj:
print nil is not Obj     // [expect] true
print 0 is not Obj       // [expect] true
print true is not Obj    // [expect] true

// is not Str
print "str" is not Str   // [expect] false
print 1 is not Str       // [expect] true

// is not Fn
// Covers Functions, Closures, BoundMethods and Natives.
fn test -> 1
cls X { ctor {} fn x -> 1 }

print (fn -> 1) is not Fn  // [expect] false
print test is not Fn       // [expect] false
print X.ctor is not Fn     // [expect] false
print X().x is not Fn      // [expect] false
print typeof is not Fn     // [expect] false

// is not Class
print X is not Class       // [expect] false
print Obj is not Class     // [expect] false
print X() is not Class     // [expect] true

// Only Classes are accepted as the rval of 'is not' operator.
print try (0 is not 1) else error // [expect] Type must be a class. Was Int.
print 0 is not typeof(1)         // [expect] false
