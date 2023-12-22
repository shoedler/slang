print 1 < 2  // [Expect] true
print 2 < 2  // [Expect] false
print 2 < 1  // [Expect] false

print 1 <= 2 // [Expect] true
print 2 <= 2 // [Expect] true
print 2 <= 1 // [Expect] false

print 1 > 2  // [Expect] false
print 2 > 2  // [Expect] false
print 2 > 1  // [Expect] true

print 1 >= 2 // [Expect] false
print 2 >= 2 // [Expect] true
print 2 >= 1 // [Expect] true

// Zero and negative zero compare the same.
print 0 < -0  // [Expect] false
print -0 < 0  // [Expect] false
print 0 > -0  // [Expect] false
print -0 > 0  // [Expect] false
print 0 <= -0 // [Expect] true
print -0 <= 0 // [Expect] true
print 0 >= -0 // [Expect] true
print -0 >= 0 // [Expect] true
