// is Num
print 0 is Num      // [expect] true
print 1.1 is Num    // [expect] true
print 0b1 is Num    // [expect] true
print 0o1 is Num    // [expect] true
print 0x1 is Num    // [expect] true
print "str" is Num  // [expect] false

// is Int
print 0 is Int      // [expect] true
print 1.1 is Int    // [expect] false
print 0x1 is Int    // [expect] true
print 0b1 is Int    // [expect] true
print 0o1 is Int    // [expect] true

// is Float
print 0 is Float    // [expect] false
print 1.1 is Float  // [expect] true
print 0x1 is Float  // [expect] false
print 0b1 is Float  // [expect] false
print 0o1 is Float  // [expect] false

// is Bool
print true is Bool   // [expect] true
print false is Bool  // [expect] true
print {} is Bool     // [expect] false

// is Nil
print nil is Nil     // [expect] true
print true is Nil    // [expect] false

// is Seq
print [] is Seq      // [expect] true
print [1, 2] is Seq  // [expect] true
print {} is Seq      // [expect] false

// is Obj
print {} is Obj      // [expect] true
print {1: 2} is Obj  // [expect] true
print [] is Obj      // [expect] false
print "" is Obj      // [expect] false
// Maybe nil, num, bool do not inherit from Obj:
print nil is Obj     // [expect] false
print 0 is Obj       // [expect] false
print true is Obj    // [expect] false

// is Str
print "str" is Str   // [expect] true
print 1 is Str       // [expect] false

// is Fn
// Covers Functions, Closures, BoundMethods and Natives.
fn test -> 1
cls X { ctor {} fn x -> 1 }

print (fn -> 1) is Fn  // [expect] true
print test is Fn       // [expect] true
print X.ctor is Fn     // [expect] true
print X().x is Fn      // [expect] true
print typeof is Fn     // [expect] true

// is Class
print X is Class       // [expect] true
print Obj is Class     // [expect] true
print X() is Class     // [expect] false

// Only Classes are accepted as the rval of 'is' operator.
print try (0 is 1) else error // [expect] Type must be a class. Was Int.
print 0 is typeof(1)         // [expect] true
