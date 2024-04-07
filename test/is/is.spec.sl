print 0 is Num      // [Expect] true
print 1.1 is Num    // [Expect] true
print 0b1 is Num    // [Expect] true
print 0o1 is Num    // [Expect] true
print 0x1 is Num    // [Expect] true
print "str" is Num  // [Expect] false

print true is Bool   // [Expect] true
print false is Bool  // [Expect] true
print {} is Bool     // [Expect] false

print nil is Nil     // [Expect] true
print true is Nil    // [Expect] false

print [] is Seq      // [Expect] true
print [1, 2] is Seq  // [Expect] true
print {} is Seq      // [Expect] false

print {} is Map      // [Expect] true
print {1: 2} is Map  // [Expect] true
print [] is Map      // [Expect] false

print "str" is Str   // [Expect] true
print 1 is Str       // [Expect] false

fn test -> 1
cls X { ctor {} fn x -> 1 }

print (fn -> 1) is Fn  // [Expect] true
print test is Fn       // [Expect] true
print X.__ctor is Fn   // [Expect] true
print X().x is Fn      // [Expect] true

print X is Class       // [Expect] true
print Map is Class     // [Expect] true
print X() is Class     // [Expect] false