// is Num
print 0 is Num      // [Expect] true
print 1.1 is Num    // [Expect] true
print 0b1 is Num    // [Expect] true
print 0o1 is Num    // [Expect] true
print 0x1 is Num    // [Expect] true
print "str" is Num  // [Expect] false

// is Int
print 0 is Int      // [Expect] true
print 1.1 is Int    // [Expect] false
print 0x1 is Int    // [Expect] true
print 0b1 is Int    // [Expect] true
print 0o1 is Int    // [Expect] true

// is Float
print 0 is Float    // [Expect] false
print 1.1 is Float  // [Expect] true
print 0x1 is Float  // [Expect] false
print 0b1 is Float  // [Expect] false
print 0o1 is Float  // [Expect] false

// is Bool
print true is Bool   // [Expect] true
print false is Bool  // [Expect] true
print {} is Bool     // [Expect] false

// is Nil
print nil is Nil     // [Expect] true
print true is Nil    // [Expect] false

// is Seq
print [] is Seq      // [Expect] true
print [1, 2] is Seq  // [Expect] true
print {} is Seq      // [Expect] false

// is Obj
print {} is Obj      // [Expect] true
print {1: 2} is Obj  // [Expect] true
print [] is Obj      // [Expect] true
print "" is Obj      // [Expect] true
// Maybe nil, num, bool do not inherit from Obj:
print nil is Obj     // [Expect] false
print 0 is Obj       // [Expect] false
print true is Obj    // [Expect] false

// is Str
print "str" is Str   // [Expect] true
print 1 is Str       // [Expect] false

// is Fn
// Covers Functions, Closures, BoundMethods and Natives.
fn test -> 1
cls X { ctor {} fn x -> 1 }

print (fn -> 1) is Fn  // [Expect] true
print test is Fn       // [Expect] true
print X.ctor is Fn     // [Expect] true
print X().x is Fn      // [Expect] true
print typeof is Fn     // [Expect] true

// is Class
print X is Class       // [Expect] true
print Obj is Class     // [Expect] true
print X() is Class     // [Expect] false

// Only Classes are accepted as the rval of 'is' operator.
print try (0 is 1) else error // [Expect] Type must be a class. Was Int.
print 0 is typeof(1)         // [Expect] true