print nil != nil // [Expect] false

print true != true // [Expect] false
print true != false // [Expect] true

print 1 != 1 // [Expect] false
print 1 != 2 // [Expect] true

print "str" != "str" // [Expect] false
print "str" != "ing" // [Expect] true

print nil != false // [Expect] true
print false != 0 // [Expect] true
print 0 != "0" // [Expect] true
